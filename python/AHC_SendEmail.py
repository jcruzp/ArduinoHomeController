import smtplib
import sys
import time

from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.MIMEBase import MIMEBase
from email import encoders
 
fromaddr = "joseacruzp@gmail.com"
toaddr = "joseacruzp@gmail.com"
emailpassword = "YOUR PASSWORD"
subject = "Arduino Home Controller by Alexa Skill"
body = "Photo taken in " + time.strftime('%a, %d %b %Y %H:%M:%S %Z(%z)')
filedir =sys.argv[1]
filename = sys.argv[2]

msg = MIMEMultipart()
 
msg['From'] = fromaddr
msg['To'] = toaddr
msg['Subject'] = subject
 
msg.attach(MIMEText(body, 'plain'))
 
attachment = open(filedir + filename, "rb")
 
part = MIMEBase('application', 'octet-stream')
part.set_payload((attachment).read())
encoders.encode_base64(part)
part.add_header('Content-Disposition', "attachment; filename= %s" % filename)
 
msg.attach(part)
 
server = smtplib.SMTP('smtp.gmail.com', 587)
server.starttls()
server.login(fromaddr, emailpassword)
text = msg.as_string()
server.sendmail(fromaddr, toaddr, text)
server.quit()
