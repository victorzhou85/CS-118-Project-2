# CS-118-Project-2: Simple Window Based Reliable Data Transfer

## Server:

### Report

Name: Andrew Tran
UID: 004188159
Email: andrewhamanhtran@gmail.com

Name: Gregory Liu 
UID: 804590076
Email: gszliu@cs.ucla.edu

Name: Zhehao Zhou
UID: 104311272
Email: viczzh@ucla.edu

I. Preface
For this project, we decided to implement the Selective Repeat protocol. We chose selective repeat because of it is more complex and robust than Go-Back-N, and also because we are more familiar with it. We decided to ACK received packets by segment #, and we also implemented a simple congestion control / slow start for extra credit.

II. Implementation

Our 1000 byte messages each contain a 16 byte header and 984 bytes of data. The header is composed of literal numbers (akin to higher level protocol headers, e.g. HTML headers).

Sender: Window Data Structure: 
typedef struct window{
  int startIndex; // the start index of the window.
  int windowLength; // the sender congestion window size
  int nextToSend; // the next first packet in the acked array who hasn’t been sent yet
  int segmentCount;	//number of segments
  int* acked; // 0: not yet sent; 1: sent but not yet acked; 2: sent
  char** segments; // segment structure, represented in the picture
  int endRTTCommand; // Indicate each wave’s last command
  int* timer; // keep track the remaining time for each packet before the timeout happens
  int* resendCommand ; // represent the packet sequence number that needs to be resend 
}* window_t;
				

Sender Functions And Routine 

We will first init the window instance, plug in all the window variables and prepare the file segment array. 

For each incoming packet on the sender function, we will follow the same routine. 
                    

Sender Side Algorithms: 

Slow Start:we have a function called updateOnAcked: this function will be fired every time there is the incoming ack. Besides the update the acked array as discussed above, it will also expand or shrink window size (windowLength) comparing to the threshold constant. 

Congestion Avoidance: When the window size reaches the threshold. the congestion avoidance requires to increase the window size on each RTT: This is another difficulties since all packets coming in sequence, which makes it pretty hard to identify each RTT. So we add another variable in the Window Struct, lastRTTCommand, storing the sequence number of the last packet of each sending command. Then each RTT ends at this particular packet get acked (will be checked at the udpateOnAcked function). 

Time Out Function:  In the selective repeat algorithm: timer should be attach to each packet. However, it is quite difficult to maintain multiple timers. What we do is depoling a periodic timer, which will fire every 1 second. Meanwhile, we have another helper array to keep tracking all the remaining time of the each packet on fly. In each second, the handler function will be called by the timer and deduct the remaining time of all the packets on fly by one second. If there is one packet, whose remaining time is zero, hasn't been acked, then the timeout algorithm will be fired. 

Corruption and Loss Handling: Corruption and loss are fabricated by declaring a random value between 0 and 1 using rand(). Corruption and loss are asserted on specific packets before sending if the random value is less then Pc or Pl. If a sender sending a data packet or a receiver sending an ack encounters a corruption, send -1 instead of the intended packet to represent a corrupted packet. If a loss segment, don’t send anything at all. Loss packets are detected by the timeout mechanism. Corrupted packets are simply ignored.

III. Difficulties

One of the biggest difficulties we faced in this project was a lack of experience with c and memory management. We spent a long time figuring out segfault errors and memory errors. We ran into difficulties in working with our segment struct and proper memory allocation for it.

Another difficulty we faced was figuring out how to implement timeouts and retransmission. When we trying to attach timers for each individual packet, it turned out that it is pretty difficult to maintain a lot of timers in C. Because of this, we decided to use only one periodic function, firing each second to checking the remaining time for the timers.

--------Data Structure---------
```C++
class window{
  private:
    int startIndex; // the start of the window.
    int windowLength; // the sender congestion window size
    int nextToSend; // the first 0 receive
    int* acked; // 0: not yet sent; 1: sent but not yet acked; 2: sent
    packets* packetArray;
  public:
    window(file* file);
    void updateOnSent();
    void updateOnAcked();
    void timeOutHandler();
    int* prepareToSend();
}

class packets{
  private
    char** segments;
  public:
  packets(file* file); // constructor:
  void sendPacket(int* sending);

}

window::window(file* file)
// Discussion: default setting to inititialize the the window property and the set off the packets constructor.  

void window::updateOnSent()
// Discussion: update the int* acked every time sender send packet

void window::updateOnAcked()
// Discussion: update the int* acked every time sender receives something. It has the congestion controller system inside, extend or shrink window size accrodingly.  

void window::timeOutHandler()
// Discussion: reset the window property, drop all other timers. start retransmission by sending the index to the sendPacket function

int* window::prepareToSend()
// Discussion: read property of window and then return the sending packet sequence number for the next blast to the packets's send packet function.

packets::packets(file* file)
// Discussion: Constructor: Array of char array with each char array 1000 bytes, allocate 988 bytes data and 12 bytes header. (order: sequence#, length, fileSize, payload)

void packets::sendPacket(int* sending)
// Discussion: given the sending index in order, send packet to socket connection, attached with timer and hander function
```
----------- Initial request-------------

