# DM365 Dev_app Project
Function:

* Receive video and Audio date from Encode
* Parse the xml message from Sip
* Send the video and aduio data to server according to the Xml message
* Do some manage work
* Receive audio data from server and write the received data to share memory
* Communicate with QT to relize the sipcall function

Note:

    1.the Audio share memory:
        dir:/shm/audio for encode to dev_app
            /shm/audio2 for dev_app to encode
        size:160 bytes
        audio:mono
    2.the video share memory: 
        dir:/shm/video/v2
        size:352*288*3/2 bytes

