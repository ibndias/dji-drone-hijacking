import socket
from drone import *
import time
import threading
import keyboard

# set up socket connection to remote C program
c_host = '192.168.1.1'
# c_host = '127.0.0.1'
c_port = 1234
c_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
c_sock.connect((c_host, c_port))
beaconpkt = send_beacon(0)
arppkt = send_arp(0)

connect_pkt         = "4080dd7d00000060e8ee64006400bc051400003200fa00f401bc05140000fa0014006400bc0514000064000101040a029ee3bf19d71333fd31433e7bb8caa1a9"
idle_pkt            = "3c80dd7d00000418e8eee8ee00000000e8eee8ee00000000e8eee8ee000000001a00551a04b10e09ce01100102000004200001084000021000045e4d"
props_pkt           = "3c80dd7d00000418d826d8260000000000ef00ef00000000f0eef0ee000000001a00551a04b10e091d03100102006c610b5b280d4000021000046968"
prefix_pkt          = "3c80dd7d00000418006900690000000020ef20ef00000000f0eef0ee000000001a00551a04b10e09a80410010200"
                                         #------#
fly_up_pkt          = prefix_pkt + "000420a50108400002100004eb79"
fly_down_pkt        = prefix_pkt + "0004205b00084000021000049809"
rot_right_pkt       = prefix_pkt + "00042000290d400002100004f253"
rot_left_pkt        = prefix_pkt + "00042000d9024000021000049d09"

                                   #------#
fly_left_pkt        = prefix_pkt + "6c01200001084000021000049340"
fly_right_pkt       = prefix_pkt + "940620000108400002100004512d"
fly_forward_pkt     = prefix_pkt + "00a434000108400002100004422a"
fly_backward_pkt    = prefix_pkt + "00640b00010840000210000467a6"

def waitresp():
    # wait for response from C program
    response = c_sock.recv(17)
    # print(response.decode())
    time.sleep(0.1) # sleep for 500 milliseconds

def tx(txpkt):
    c_sock.send('-'.encode()+(len(bytes_hex(txpkt))).to_bytes(2, byteorder='big')+bytes_hex(txpkt))
    waitresp()

def beacon():
    tx(beaconpkt)

def arp():
    tx(arppkt)

def control(movement):
    udppkt = send_udp(movement,2)
    tx(udppkt)

def connect():
    # CONNECT
    udppkt = send_udp(connect_pkt,1)
    tx(udppkt)

    control(idle_pkt)

# Set up global variable to track toggle state
beacons = False

def beacons_background_function():
    while True:
        if beacons:
            # Do something in the background here
            # print("beacons thread is running...")
            beacon()
            control(idle_pkt)
            time.sleep(0.2) 

# Create a new thread for the background function
background_thread = threading.Thread(target=beacons_background_function)

# Start the thread
background_thread.start()

print("SENDING PACKET =========================>>>>>>>>>>>>>>>>>>")
# while True:

while True:
    print("Beacons Status: ")
    print(beacons)
    print("Please choose \nc Connect\nx Turn On Propeller\nw Fly Up\nz Float\ns Land\nd Right\na Left\n")
    # choice = getch.getche()
    choice = keyboard.read_key()
    if choice == 'c':
        print("CONNECTING =========================>>>>>>>>>>>>>>>>>>")
        beacon()
        arp()
        beacon()
        connect()
        beacon()
        beacons = True
    elif choice == 'x':
        print("Turning on propeller =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(props_pkt)
    elif choice == 'w':
        print("Flying up =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(fly_up_pkt)
    elif choice == 'e':
        print("Floating =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(idle_pkt)
    elif choice == 's':
        print("Down =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(fly_down_pkt)
    elif choice == 'd':
        print("Rotate Right =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(rot_right_pkt)
    elif choice == 'a':
        print("Rotate Left =========================>>>>>>>>>>>>>>>>>>")
        beacons = False
        control(rot_left_pkt)
    else:
        print("Invalid choice, please try again")
        continue

    beacons = True
    time.sleep(0.1)
