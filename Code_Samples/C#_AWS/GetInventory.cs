// GetInventory

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Text; // StringBuilder
using System.Net.Mail;
// requires references AWSSDK.Core and AWSSDK.Glacier
using Amazon.Glacier;
using Amazon.Glacier.Model;
using Amazon.Glacier.Transfer;
using Amazon.Runtime;
// requires reference to AWSSDK.SimpleNotificationService
using Amazon.SimpleNotificationService;
using Amazon.SimpleNotificationService.Model;
// requires reference to AWSSDK.SQS
using Amazon.SQS;
using Amazon.SQS.Model;
// requires reference to Newtonsoft.Json
using Newtonsoft.Json;

// Note:  I can't use
// using System.Threading.Tasks
// because I can't get VS2010 to go higher than 3.5 .NET Framework.


namespace GetInventory
{
    class Program
    {
        static string s_senderAccount = "auto@company.com";
        static string s_senderPassword = Credentials.GetPassword();
        static string s_recipientAccount = "auto@company.com";

        static string s_vaultName = "Company_Backup";

        static string s_path = @"D:\Company_Backup_Inventory\";

        static string s_filePathTemplate = Path.Combine(s_path, s_vaultName + "_Vault_Inventory_");
        static string s_messagePathTemplate = Path.Combine(s_path, s_vaultName + "_Vault_Inventory_Message_");

        static string s_topicArn;
        static string s_queueUrl;
        static string s_queueArn;
        static string s_filePath;

        static AmazonSimpleNotificationServiceClient s_snsClient;
        static AmazonSQSClient s_sqsClient;
        const string SQS_POLICY =
            "{" +
                " \"Version\" : \"2012-10-17\"," +
                " \"Statement\" : [" +
                " {" +
                " \"Sid\" : \"sns-rule\"," +
                " \"Effect\" : \"Allow\"," +
                " \"Principal\" : \"*\"," +
                " \"Action\" : \"sqs:SendMessage\"," +
                " \"Resource\" : \"{QuernArn}\"," +
                " \"Condition\" : {" +
                " \"ArnLike\" : {" +
                " \"aws:SourceArn\" : \"{TopicArn}\"" +
                " }" +
                " }" +
                " }" +
                " ]" +
            "}";

        class Archive
        {
            public string ArchiveId { get; set; }
            public string ArchiveDescription { get; set; }
            public string CreationDate { get; set; }
            public string Size { get; set; }
            public string SHA256TreeHash { get; set; }
        }

        class VaultInventory
        {
            public string VaultARN { get; set; }
            public string InventoryDate { get; set; }
            public List<Archive> ArchiveList { get; set; }
        }

        static StringBuilder s_log = new StringBuilder();


        static void Main(string[] args)
        {
            // Define a delegate that calls GetInventory()
            Func<Amazon.RegionEndpoint, bool> action = (Amazon.RegionEndpoint region) =>
            {
                return GetInventory(region);
            };


            bool error = false;

            DateTime startDateTime = DateTime.Now;
            string strStartDateTime = startDateTime.ToString("yyyy-MM-dd_HH-mm-ss");
            s_filePath = s_filePathTemplate + strStartDateTime + ".txt";
            s_messagePathTemplate += strStartDateTime + ".txt";

            WriteLogConsole("Started: " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));

            // Amazon.RegionEndpoint.USEast1 is Virginia
            // Amazon.RegionEndpoint.USWest2 is Oregon

            try
            {
                Amazon.RegionEndpoint region = Amazon.RegionEndpoint.USEast1;
                WriteLogConsole(region.ToString() + ": " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                s_filePath = s_filePathTemplate + region.ToString() + "_" + strStartDateTime + ".txt";

                error = GetInventory(region);
                region = Amazon.RegionEndpoint.USWest2;
                WriteLogConsole(region.ToString() + ": " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                s_filePath = s_filePathTemplate + region.ToString() + "_" + strStartDateTime + ".txt";

                WriteLogConsole(region.ToString() + ": " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));

                bool secondError = GetInventory(region);
                if (secondError)
                    error = true;
            }
            catch (AmazonGlacierException e)
            {
                WriteLogConsole("AmazonGlacierException: " + e.Message);
                error = true;
            }
            catch (AmazonServiceException e)
            {
                WriteLogConsole("AmazonServiceException: " + e.Message);
                error = true;
            }
            catch (Exception e)
            {
                WriteLogConsole("Exception: " + e.Message);
                error = true;
            }

            DateTime endDateTime = DateTime.Now;
            WriteLogConsole("End time: " + endDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

            TimeSpan timeTaken = endDateTime - startDateTime;
            WriteLogConsole("Time taken: " + timeTaken.ToString());

            Email((error ? "Error: " : "") + "Amazon Vault Inventory", s_log.ToString());
        }


        static bool GetInventory(Amazon.RegionEndpoint region)
        {
            bool error = false;

            AmazonGlacierClient client;
            try
            {
                using (client = new AmazonGlacierClient(region))
                {
                    WriteLogConsole("Setup SNS topic and SQS queue.", region);
                    SetupTopicAndQueue(region);
                    WriteLogConsole("Retrieve Inventory List", region);
                    GetVaultInventory(client, region);
                }
                WriteLogConsole("Operations successful.", region);
                DisplayVaultInventory();
            }
            catch (AmazonGlacierException e)
            {
                WriteLogConsole("AmazonGlacierException: " + e.Message, region);
                error = true;
            }
            catch (AmazonServiceException e)
            {
                WriteLogConsole("AmazonServiceException: " + e.Message, region);
                error = true;
            }
            catch (Exception e)
            {
                WriteLogConsole("Exception: " + e.Message, region);
                error = true;
            }
            finally
            {
                // Delete SNS topic and SQS queue.
                s_snsClient.DeleteTopic(new DeleteTopicRequest() { TopicArn = s_topicArn });
                s_sqsClient.DeleteQueue(new DeleteQueueRequest() { QueueUrl = s_queueUrl });
            }

            return error;
        }


        static void SetupTopicAndQueue(Amazon.RegionEndpoint region)
        {
            long ticks = DateTime.Now.Ticks;

            // Setup SNS topic.
            s_snsClient = new AmazonSimpleNotificationServiceClient(region);
            s_sqsClient = new AmazonSQSClient(region);

            s_topicArn = s_snsClient.CreateTopic(new CreateTopicRequest
                { Name = "GlacierDownload-" + ticks }).TopicArn;

            WriteLogConsole("topicArn: " + s_topicArn);

            CreateQueueRequest createQueueRequest = new CreateQueueRequest();
            createQueueRequest.QueueName = "GlacierDownload-" + ticks;
            CreateQueueResponse createQueueResponse = s_sqsClient.CreateQueue(createQueueRequest);
            s_queueUrl = createQueueResponse.QueueUrl;

            WriteLogConsole("QueueURL: " + s_queueUrl);

            GetQueueAttributesRequest getQueueAttributesRequest = new GetQueueAttributesRequest();
            getQueueAttributesRequest.AttributeNames = new List<string> { "QueueArn" };
            getQueueAttributesRequest.QueueUrl = s_queueUrl;

            GetQueueAttributesResponse response = s_sqsClient.GetQueueAttributes(getQueueAttributesRequest);
            s_queueArn = response.QueueARN;
            WriteLogConsole("QueueArn: " + s_queueArn);

            // Setup the Amazon SNS topic to publish to the SQS queue.
            s_snsClient.Subscribe(new SubscribeRequest()
                {
                    Protocol = "sqs",
                    Endpoint = s_queueArn,
                    TopicArn = s_topicArn
                });

            // Add the policy to the queue so SNS can send messages to the queue.
            var policy = SQS_POLICY.Replace("{TopicArn}", s_topicArn).Replace("{QuernArn}", s_queueArn);
            s_sqsClient.SetQueueAttributes(new SetQueueAttributesRequest()
                {
                    QueueUrl = s_queueUrl,
                    Attributes = new Dictionary<string, string>
                    {
                        { QueueAttributeName.Policy, policy }
                    }
                });
        }


        static void GetVaultInventory(AmazonGlacierClient client, Amazon.RegionEndpoint region)
        {
            // Initiate job.
            InitiateJobRequest initJobRequest = new InitiateJobRequest()
            {
                VaultName = s_vaultName,
                JobParameters = new JobParameters()
                {
                    Type = "inventory-retrieval",
                    Description = "This job is to download a vault inventory.",
                    SNSTopic = s_topicArn,
                }
            };

            InitiateJobResponse initJobResponse = client.InitiateJob(initJobRequest);
            string jobId = initJobResponse.JobId;

            // Check queue for a message and if job completed successfully, download inventory.
            ProcessQueue(jobId, client, region);
        }

        private static void ProcessQueue(string jobId, AmazonGlacierClient client, Amazon.RegionEndpoint region)
        {
            ReceiveMessageRequest receiveMessageRequest = new ReceiveMessageRequest()
                { QueueUrl = s_queueUrl, MaxNumberOfMessages = 1 };
            bool jobDone = false;

            while (!jobDone)
            {
                WriteLogConsole("Poll SQS queue");
                ReceiveMessageResponse receiveMessageResponse
                    = s_sqsClient.ReceiveMessage(receiveMessageRequest);

                if (receiveMessageResponse.Messages.Count == 0)
                {
                    Thread.Sleep(10000 * 60);
                    continue;
                }

                WriteLogConsole("Got message");

                Message message = receiveMessageResponse.Messages[0];
                Dictionary<string, string> outerLayer
                    = JsonConvert.DeserializeObject<Dictionary<string, string>>(message.Body);
                Dictionary<string, object> fields
                    = JsonConvert.DeserializeObject<Dictionary<string, object>>(outerLayer["Message"]);

                string statusCode = fields["StatusCode"] as string;
                if (string.Equals(statusCode, GlacierUtils.JOB_STATUS_SUCCEEDED,
                    StringComparison.InvariantCultureIgnoreCase))
                {
                    WriteLogConsole("Downloading job output");
                    DownloadOutput(jobId, client, region); // Save job output to the specified file location.
                }
                else if (string.Equals(statusCode, GlacierUtils.JOB_STATUS_FAILED,
                    StringComparison.InvariantCultureIgnoreCase))
                    WriteLogConsole("Job failed... cannot download the inventory.");

                jobDone = true;
                s_sqsClient.DeleteMessage(new DeleteMessageRequest()
                    {
                        QueueUrl = s_queueUrl,
                        ReceiptHandle = message.ReceiptHandle
                    });
            }
        }


        private static void DownloadOutput(string jobId, AmazonGlacierClient client, Amazon.RegionEndpoint region)
        {
            GetJobOutputRequest getJobOutputRequest = new GetJobOutputRequest()
                {
                    JobId = jobId,
                    VaultName = s_vaultName
                };
            GetJobOutputResponse getJobOutputResponse = client.GetJobOutput(getJobOutputRequest);

            using (Stream webStream = getJobOutputResponse.Body)
            {
                using (Stream fileToSave = File.OpenWrite(s_filePath))
                {
                    CopyStream(webStream, fileToSave);
                }
            }
        }

        public static void CopyStream(Stream input, Stream output)
        {
            byte[] buffer = new byte[65536];
            int length;
            while ((length = input.Read(buffer, 0, buffer.Length)) > 0)
            {
                output.Write(buffer, 0, length);
            }
        }

        public static void DisplayVaultInventory()
        {
            string json = File.ReadAllText(s_filePath);

            var results = JsonConvert.DeserializeObject<dynamic>(json);
            /* "Predefined type 'Microsoft.CSharp.RuntimeBinder.Binder' is not defined or imported"
            Console.WriteLine(results.VaultARN);
            Console.WriteLine(results["VaultARN"]);
            */
            // Seems to require .NET 4, which is not supported in VS2010
            // "Predefined type 'Microsoft.CSharp.RuntimeBinder.Binder' is not defined or imported"
            // Newtonsoft.Json.Linq.JObject results = JsonConvert.DeserializeObject<dynamic>(json);

            VaultInventory vaultInventory = JsonConvert.DeserializeObject<VaultInventory>(json);

            int archiveCount = 0;

            WriteLogConsole("Vault Inventory:");
            WriteLogConsole("VaultARN: " + vaultInventory.VaultARN);
            WriteLogConsole("InventoryDate: " + vaultInventory.InventoryDate);
            WriteLogConsole("");
            WriteLogConsole("ArchiveList:");
            WriteLogConsole("");
            foreach (Archive archive in vaultInventory.ArchiveList)
            {
                WriteLogConsole("Name\t" + archive.ArchiveDescription);
                WriteLogConsole("ID\t" + archive.ArchiveId);
                WriteLogConsole("CreationDate\t'" + archive.CreationDate);
                WriteLogConsole("Size\t" + string.Format("{0:N2}", Int64.Parse(archive.Size) / 1024.0 / 1024.0 / 1024.0));
                WriteLogConsole("");

                ++archiveCount;
            }

            WriteLogConsole(string.Format("Archive count: {0}", archiveCount));
        }


        static void Email(string subject, string body)
        {
            try
            {
                MailMessage mail = new MailMessage();
                SmtpClient smtpServer = new SmtpClient("smtp.smarshconnect.com");

                mail.From = new MailAddress(s_senderAccount);
                mail.To.Add(s_recipientAccount);
                mail.Subject = subject;
                mail.Body = body;

                SmtpClient SmtpServer = new SmtpClient("smtp.smarshconnect.com");
                SmtpServer.Port = 587;
                SmtpServer.Credentials = new System.Net.NetworkCredential(s_senderAccount, s_senderPassword);
                SmtpServer.EnableSsl = true;

                SmtpServer.Send(mail);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception: " + Environment.NewLine + ex.ToString());
                // Do not throw this exception because the backup process should continue
                // even if email is not working.
            }
        }

        static void WriteLogConsole(string s, Amazon.RegionEndpoint region=null)
        {
            s_log.Append((region == null ? "" : region.ToString() + ": ") + s + Environment.NewLine);
            Console.WriteLine(s);
        }
    }
}
