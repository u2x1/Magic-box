 # MP3hider

Hide any data into a mp3 file by inserting unplayable frames before actual audio data. 

> Notice: It's _*NOT*_ a steganography tool and it's not safe at all.

## Usage ##

#### Hiding data ####

` hider carrier_file secret_file`

This will generate a mp3 file named "output.mp3". The file saves the extension name and the size of secret file.

#### Retrieving hidden data ####

`hider carrier_file`

This will decrypt the carrier file with file extension.

---

e.g

`hider input.mp3 test.png` ==> `output.mp3`
`hider output.mp3` ==> `secret.png`
