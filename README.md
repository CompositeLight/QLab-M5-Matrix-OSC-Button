# QLab-M5-Matrix-OSC-Button
Project to turn an M5 Atom Matrix into a wifi GO button for use with QLab

Important note: This is pretty much My First Coding Project, so likely full of issues! Any good bits have been magpied from the Genius of Joseph Adams, as I've used his M5 Tally Arbiter listener as a starting point.

# Setup:
1) IMPORTANT: You need to set your QLab preferences to require 0.2 seconds between GO commands - explained below.
2) Edit the code to insert your SSID name and password, along with your QLab Mac's IP address.
3) Once uploaded, the M5 will attempt to connect to wifi. On connection it will send a THUMP to QLab to confirm there is a workspace on the network.
4) Once the M5 goes green, it's ready to send.

# Operation:
On power up the M5 will attempt to connect to wifi. You will see a blue cross until it connects.
On network connection it will send a THUMP to QLab to confirm there is a workspace on the network. If there is no resonse, you'll see a red cross.
Once the M5 goes green, it's ready to send.

When the button is pressed, the M5 will send a "GO", wait for 0.1 seconds, and send a second "GO".
This double send is intended to avoid the issue that, every so often, a UDP command doesn't get through.
The M5 will briefly turn red to confirm the command was sent.

Every 30 seconds (by default) the M5 will send a THUMP to QLab to check for response. It also checks after every button GO action. I think, if you pressed the button right as the THUMP check was taking place, the go would be lost - though I haven't actually managed to make this happen. It would probably make sense to have the THUMP check running as a seperate thread, but I'm currently not that clever!

Hope this is helpful to someone. Good luck out there!

