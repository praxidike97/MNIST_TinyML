import requests

url = 'http://192.168.178.86/image-upload.php'
files = {'fileToUpload': open('/Users/fabian/Pictures/raspberry-pi-logo.png', 'rb')}
r = requests.post(url, files=files)
print(r)