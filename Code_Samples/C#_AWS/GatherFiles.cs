// GatherFiles

using System;
using System.Collections.Generic; // List
using System.Text; // StringBuilder
using System.IO; // Path
using System.Threading; // Sleep
using System.Data.SqlClient;
using System.Net.Mail;
using System.Diagnostics;

// This program assumes the drive mapping configuration that is expressed in s_sourceFolders.
// I have created a graphic of the mappings that should accompany this code, see
// "Drive mappings.png".


namespace GatherFiles
{
    class Program
    {
        static string s_7zPassword = Credentials.Get7zPassword();

        static string s_destDrive = @"D:\";
        static string s_destFolder = "Company_backup_";
        static string s_destFolderPath = "";

        static string s_7z_flags = string.Format("a -t7z -p{0} -r -mx=9 -myx=9 -m0=LZMA2", s_7zPassword);
        static string s_7zFilePath = "\"" + @"C:\Program Files\7-Zip\7z.exe" + "\"";


        // An ArchivePart is a set of files in a folder
        class ArchivePart
        {
            public List<string> _files = new List<string>();
            public string SourceFolder { get; set; }
            public string Drive { get; set; }
            public decimal SizeGBs { get; set; } // in GBs

            public ArchivePart(string sourceFolder, string drive) { SourceFolder = sourceFolder; Drive = drive; SizeGBs = 0; }
        }

        class Archive
        {
            public List<ArchivePart> _parts = new List<ArchivePart>();
            public decimal SizeGBs  // in GBs
            {
                get
                {
                    decimal sizeGBs = 0;
                    foreach (ArchivePart part in _parts)
                        sizeGBs += part.SizeGBs;
                    return sizeGBs;
                }
            
            }
            public int FileCount
            {
                get
                {
                    int fileCount = 0;
                    foreach (ArchivePart part in _parts)
                        fileCount += part._files.Count;
                    return fileCount;
                }
            }
            public List<string> Files
            {
                get
                {
                    List<string> files = new List<string>();
                    foreach (ArchivePart part in _parts)
                        files.AddRange(part._files);
                    return files;
                }
            }

            public Archive(List<ArchivePart> parts) { _parts = parts; }
        }

        static Dictionary<string, Archive> s_archives = new Dictionary<string, Archive>()
            {
                /* applications is so big that is causes network timeouts when I try to upload it.
                 * Therefore it must be split into smaller pieces.  Thus it must be handled separately.
                 * See GetApplicationsSources().

                { "Company_applications_",
                    new Archive(new List<ArchivePart> { new ArchivePart("applications", @"F:\") } ) },
                */
                { "Company_development_",
                    new Archive(new List<ArchivePart> { new ArchivePart("development", @"G:\") } ) },
                { "Company_apx_", 
                    new Archive(
                        new List<ArchivePart> {
                            new ArchivePart("apx-db", @"H:\"), new ArchivePart("apx-app", @"I:\") } ) },
                { "Company_eze_",
                    new Archive(
                        new List<ArchivePart> {
                            new ArchivePart("eze-db", @"J:\"), new ArchivePart("eze-app", @"K:\") } ) },
                { "Company_kentfield_",
                    new Archive(new List<ArchivePart> { new ArchivePart("kentfield", @"L:\") } ) },
                { "Company_tiburon_", 
                    new Archive(new List<ArchivePart> { new ArchivePart("tiburon", @"M:\") } ) },
            };

        static Dictionary<string, Archive> s_applicationsArchives = new Dictionary<string, Archive>();

        static bool s_userInput = true;


        static void Main(string[] args)
        {
            DateTime startDateTime = DateTime.MinValue;
            
            try
            {
                DateTime yearMonthDate = DateTime.MinValue;

                // Any command line parameters will initiate silent mode that gathers the file for previous work day.
                String[] arguments = Environment.GetCommandLineArgs();
                if (arguments.Length > 1)
                {
                    DateTime previousWorkDayDateTime = GetPreviousWorkDay(DateTime.Now);
                    yearMonthDate = new DateTime(previousWorkDayDateTime.Year, previousWorkDayDateTime.Month, previousWorkDayDateTime.Day);

                    s_userInput = false;
                }
                else
                {
                    string year, month, date;
                    if (!GetYearMonthDate(out year, out month, out date))
                        return;

                    yearMonthDate = new DateTime(Int32.Parse(year), Int32.Parse(month), Int32.Parse(date));
                }

                SetStatics(yearMonthDate);
                    
                CreateFolder(yearMonthDate);

                if (!GetSources(yearMonthDate))
                    return;

                startDateTime = DateTime.Now;
                Console.WriteLine("Start time: " + startDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                // If more about 6 processes run then some of them fail silently.
                // So break up the process into multiple batches.
                CreateArchives(s_archives, yearMonthDate);
                CheckArchives(s_archives);

                CreateArchives(s_applicationsArchives, yearMonthDate);
                CheckArchives(s_applicationsArchives);
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }

            DateTime endDateTime = DateTime.Now;
            Console.WriteLine("End time: " + endDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

            TimeSpan timeTaken = endDateTime - startDateTime;
            Console.WriteLine("Time taken: " + timeTaken.ToString());

            if (s_userInput)
            {
                Console.WriteLine("Press a key to exit");
                Console.ReadKey();
            }
        }


        static void CheckArchives(Dictionary<string, Archive> archives)
        {
            foreach (string archive in archives.Keys)
            {
                if (archives[archive].FileCount < 1)
                    continue;

                foreach (string filePath in archives[archive].Files)
                    if (!File.Exists(filePath))
                        Console.WriteLine("{0} does NOT exist", filePath);
            }
        }


        static void CreateArchives(Dictionary<string, Archive> archives, DateTime yearMonthDay)
        {
            List<Process> processes = new List<Process>();
            Dictionary<Process, string> processNames = new Dictionary<Process, string>();

            Console.WriteLine("");
            Console.WriteLine("CreateArchives");
            Console.WriteLine("");

            foreach (string archive in archives.Keys)
            {
                if (archives[archive].FileCount < 1)
                    continue;

                Console.WriteLine("Archive: {0}", archive);

                Process process = CreateArchive(archive, archives[archive].Files);
                processes.Add(process);
                processNames[process] = archive;

                Console.WriteLine("");
            }

            foreach (Process process in processes)
            {
                Console.WriteLine("{0}: Start() process {1}", DateTime.Now.ToString(), processNames[process]);
                // throws
                // Console.WriteLine("Start() on {0}", process.ProcessName);
                process.Start();

                // This program fails to create archives even though the console output show their creation.
                // This is code is a guess as to a solution.  Wait after each processes starts, to 
                // decrease the chances that they stop on eachother.
                Thread.Sleep(5000);
            }

            // The following code has the intended effect of waiting until all 7z processes have completed
            // so that this program can accurately time how long the entire procedure took.
            //
            // Without a WaitForExit(), execution continues while the other processes runs asynchronously,
            // which prevents timing the work.
            //
            // Each call to WaitForExit() blocks, which is what we want.  However, I was concerned that
            // this program may block waiting on a process A before this program can call WaitForExit()
            // for process B.  But it doesn't matter if a process completes before WaitForExit() is called.

            foreach (Process process in processes)
            {
                Console.WriteLine("{0}: WaitForExit() on process {1}", DateTime.Now.ToString(), processNames[process]);
                // throws
                // Console.WriteLine("WaitForExit() on {0}", process.ProcessName);
                process.WaitForExit();
            }

            Console.WriteLine("{0}: All processes done", DateTime.Now.ToString());
            Console.WriteLine("");
        }

        static Process CreateArchive(string archive, List<string> files)
        {
            foreach (string file in files)
                Console.WriteLine(file);

            Process process = new Process();
            ProcessStartInfo startInfo = new ProcessStartInfo();
            // startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;

            startInfo.FileName = s_7zFilePath;

            StringBuilder arguments = new StringBuilder(s_7z_flags + " " + "\"" + Path.Combine(s_destFolderPath, archive) + "\"");
            // StringBuilder arguments = new StringBuilder("/C a -t7z -pm1adisoN! -r -mx=9 -myx=9 -m0=LZMA2 " + "\"" + Path.Combine(s_destFolderPath, s_7zFileName) + "\"");
            foreach (string sourcePath in files)
                arguments.Append(" \"" + sourcePath + "\"");

            startInfo.Arguments = arguments.ToString();

            Console.WriteLine("command: " + startInfo.FileName);
            Console.WriteLine("arguments: " + startInfo.Arguments);

            process.StartInfo = startInfo;

            return process;
        }


        static void GetApplicationsSources(DateTime yearMonthDay)
        {
            string applicationsDrive = @"F:\";
            string applicationsFolder = "applications";

            // assume applications sources are at drive F:
            foreach (string filePath in GetFiles(applicationsDrive, yearMonthDay))
            {
                if (!filePath.StartsWith(Path.Combine(applicationsDrive, applicationsFolder))
                    || !filePath.EndsWith(".tar.bz2"))
                    continue;

                ArchivePart part = new ArchivePart(applicationsFolder, applicationsDrive);
                part._files = new List<string> { filePath };

                Archive archive = new Archive( new List<ArchivePart> { part });

                string archiveName = "Company_" + filePath.Substring(applicationsDrive.Length) + ".7z";

                s_applicationsArchives[archiveName] = archive;
            }
        }


        static bool GetSources(DateTime yearMonthDay)
        {
            // count all the files in all the archives and the total size
            int fileCount = 0;
            decimal sizeGBs = 0;

            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName]; // throws is index is not found
                foreach (ArchivePart part in archive._parts)
                {
                    part._files.AddRange(GetFiles(part.Drive, yearMonthDay));
                    // Compute archive size
                    foreach (string filePath in part._files)
                    {
                        double size = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        // review what is the more responsible conversions
                        part.SizeGBs += (decimal) size;
                        sizeGBs += (decimal)size;
                    }
                    fileCount += part._files.Count;
                }
            }

            // handle the applications case
            GetApplicationsSources(yearMonthDay);
            foreach (string archiveName in s_applicationsArchives.Keys)
            {
                Archive archive = s_applicationsArchives[archiveName]; // throws is index is not found
                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double size = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        // review what is the more responsible conversions
                        part.SizeGBs += (decimal)size;
                        sizeGBs += (decimal)size;
                    }
                    fileCount += part._files.Count;
                }
            }

            Console.WriteLine("");
            Console.WriteLine(string.Format(
                "{0} archives containing a total of {1} files with a total size of {2:N2} GB",
                s_archives.Keys.Count + s_applicationsArchives.Count, fileCount, sizeGBs));
            Console.WriteLine("");

            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName]; // throws is index is not found
                Console.WriteLine(string.Format(
                    "\"{0}\": {1} files, size {2:N2} GB", archiveName, archive.FileCount, archive.SizeGBs));

                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double fileSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        Console.WriteLine(string.Format(
                            "\"{0}\": size {1:N2} GB", filePath, fileSizeInGigabytes));
                    }
                }
                Console.WriteLine("");
            }

            foreach (string archiveName in s_applicationsArchives.Keys)
            {
                Archive archive = s_applicationsArchives[archiveName]; // throws is index is not found
                Console.WriteLine(string.Format(
                    "\"{0}\": {1} files, size {2:N2} GB", archiveName, archive.FileCount, archive.SizeGBs));

                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double fileSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        Console.WriteLine(string.Format(
                            "\"{0}\": size {1:N2} GB", filePath, fileSizeInGigabytes));
                    }
                }
                Console.WriteLine("");
            }

            if (s_userInput)
            {
                Console.Write("Continue?  'y' for yes and 'q' for quit >");
                string reply = Console.ReadLine();

                return reply == "y";
            }
            else
                return true;
        }


        static List<string> GetFiles(string path, DateTime yearMonthDay)
        {
            string dash = yearMonthDay.Year.ToString("D4") + "-" + yearMonthDay.Month.ToString("D2") + "-" + yearMonthDay.Day.ToString("D2");
            string underline = yearMonthDay.Year.ToString("D4") + "_" + yearMonthDay.Month.ToString("D2") + "_" + yearMonthDay.Day.ToString("D2");

            short count = 0;

            List<string> files = new List<String>();

            string[] fileEntries = Directory.GetFiles(path);
            foreach (string fileName in fileEntries)
                if (fileName.Contains(dash) || fileName.Contains(underline))
                {
                    // 2016-08-23 Tiburon is now generating extra .7z files to be uploaded to manhattan.
                    // Ignore them.
                    if (fileName.IndexOf(".7z") == fileName.Length - 3)
                        continue;

                    files.Add(Path.Combine(path, fileName));
                    ++count;
                }

            if (count == 0)
                Console.WriteLine("ERROR:  No matching files found in " + path);

            return files;
        }


        static void CreateFolder(DateTime yearMonthDay)
        {
            if (Directory.Exists(s_destFolderPath))
                throw new Exception(string.Format("Folder \"{0}\" already exists", s_destFolderPath));

            DirectoryInfo di = Directory.CreateDirectory(s_destFolderPath);
            Console.WriteLine("");
            Console.WriteLine("Created folder \"{0}\"", s_destFolderPath);
        }


        static bool GetYearMonthDate(out string year, out string month, out string date)
        {
            string reply = "";

            year = "";
            month = "";
            date = "";

            do
            {
                try
                {
                    Console.Write("Enter year: ");
                    year = Console.ReadLine();

                    Console.Write("Enter month (1-12) with leading zero if necessary: ");
                    month = Console.ReadLine();

                    Console.Write("Enter date (1-31) with leading zero if necessary: ");
                    date = string.Format("{0:00}", Console.ReadLine());

                    Console.WriteLine("Is this date correct? {0}-{1}-{2}", year, month, date);
                    Console.Write("Type 'y' for yes, 'n' for no, or 'q' or quit >");

                    reply = Console.ReadLine();
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception: " + e.Message);
                }
            }
            while (reply != "y" && reply != "q");

            return reply != "q";
        }



        static DateTime GetPreviousWorkDay(DateTime dateTime)
        {
            DateTime previousWorkDay = DateTime.MinValue;

            string connectionString = "Data Source=tiburon;Initial Catalog=Napa";
            SqlConnection cnn = new SqlConnection(connectionString);
            cnn.Open();

            string sql = string.Format("select max(substring(CONVERT(varchar,CalendarDate,111),1,12)) as CalendarDate from CalendarDays where CalendarDate < '{0}'",
                dateTime.ToString("yyyy-MM-dd"));
            SqlCommand command = new SqlCommand(sql, cnn);

            SqlDataReader dataReader = command.ExecuteReader();
            if (dataReader.Read())
            {
                string stringPreviousWorkDay = dataReader.GetValue(0).ToString();
                int year = 0;
                int month = 0;
                int date = 0;
                GetYearMonthDate(stringPreviousWorkDay, ref year, ref month, ref date);
                previousWorkDay = new DateTime(year, month, date);
            }

            dataReader.Close();
            command.Dispose();
            cnn.Close();

            if (previousWorkDay == DateTime.MinValue)
                throw new Exception(string.Format("No return value for previous work day of '{0}'", dateTime.ToString("yyyy-MM-dd")));

            return previousWorkDay;
        }

        // YYYY-MM-DD or YYYY/MM/DD
        static void GetYearMonthDate(string yearMonthDate, ref int year, ref int month, ref int date)
        {
            year = Int32.Parse(yearMonthDate.Substring(0, 4));
            month = Int32.Parse(yearMonthDate.Substring(5, 2));
            date = Int32.Parse(yearMonthDate.Substring(8, 2));
        }

        static void SetStatics(DateTime yearMonthDate)
        {
            // append the date to s_archives keys
            Dictionary<string, Archive> newArchives = new Dictionary<string, Archive>();
            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName];
                string newArchiveName = archiveName + yearMonthDate.ToString("yyyy-MM-dd") + ".7z";
                newArchives[newArchiveName] = archive;
            }
            s_archives = newArchives;

            s_destFolder = s_destFolder + yearMonthDate.ToString("yyyy-MM-dd");
            s_destFolderPath = Path.Combine(s_destDrive, s_destFolder);

            Console.WriteLine("Destination directory path: " + s_destFolderPath);
            Console.WriteLine("");
        }
    }
}
