# Import smtplib for the actual sending function
import smtplib

# Import the email modules we'll need
from email.mime.text import MIMEText

g_mailFrom = "sender@company.com"
g_mailToError = ['user1@company.com', 'user2@company.net']
g_mailToNotice = ['user3@company.com']


def Mail(subject, body, isError=True):
	if isError:
		return MailTo(g_mailToError, subject, body)
	else:
		return MailTo(g_mailToNotice, subject, body)


# On success, an empty string is returned.  On error, an error string.
# to is a list of strings, subject and body are strings
def MailTo(to, subject, body):
	# Create a text/plain message
	msg = MIMEText(body)
	msg['Subject'] = subject
	msg['From'] = g_mailFrom
	msg['To'] = "; ".join(to)
	
	try:
		s = smtplib.SMTP('smtp.smarshconnect.com:587')
		s.login(g_mailFrom, g_mailPassword)
		s.sendmail(g_mailFrom, to, msg.as_string())
	except Exception as e:
		err = "mail.Mail(): Exception caught: " + repr(e)
		return err
	finally:
		s.quit()
		
	return ""


