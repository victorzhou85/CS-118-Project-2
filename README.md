# CS-118-Project-2

## Server:

--------Data Structure---------

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


--------Member Function---------

window::window(file* file)
--Discussion: default setting to inititialize the the window property and the set off the packets constructor.  

void window::updateOnSent()
-- Discussion: update the int* acked every time sender send packet

void window::updateOnAcked()
-- Discussion: update the int* acked every time sender receives something. It has the congestion controller system inside, extend or shrink window size accrodingly.  

void window::timeOutHandler()
-- Discussion: reset the window property, drop all other timers. start retransmission by sending the index to the sendPacket function

int* window::prepareToSend()
-- Discussion: read property of window and then return the sending packet sequence number for the next blast to the packets's send packet function.

packets::packets(file* file)
-- Discussion: Constructor: Array of char array with each char array 1000 bytes, allocate 988 bytes data and 12 bytes header. (order: sequence#, length, fileSize, payload)

void packets::sendPacket(int* sending)
-- Discussion: given the sending index in order, send packet to socket connection, attached with timer and hander function
