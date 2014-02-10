#include <Client.h>
#include <Ethernet.h>
#include <Server.h>
#include <Udp.h>
#include <SPI.h>
//#include <Agentuino.h>   //snmp ...we'll test this another day.

#define VERSION "0.1"

//This code was used for a gate. Change this to whatever you're working with
//to make it more appropriate.

//Define the gate and pin values

#define gateopen 2
#define gateclose 3
#define gatestop 4
#define gatestatus 9

// Version 0.1  - Initial code write up.
// Had to change the gate status level because the contact I had was normally closed (open with the magnetic there) instead of normally open (closed with the magnetic near by). Its what I had around...
/*
 * Relay Controller
 * Provide a web interface to display gate status and the ability to open, stop or close.
 * 
 */
 //-----------------------BEGIN Variable setup -------------------------------
 
String readString = String(" "); //string for fetching data from address
boolean reading = false;

unsigned long time;  //Use this for time stamp for serial/debug logging info.

int gatecommand=0;

int state;
int val=0;
 
// hardset MAC address 
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  
byte ip[] =  {
  10, 0, 0, 5 }; //This is the fail back IP if DHCP doesn't get a valid address.
byte gateway[] = { 10, 0, 0, 1 }; //Manual setup only
byte subnet[] = { 255, 255, 255, 0 }; //Manual setup only


EthernetServer server(80);
//Server server(80); //Standard HTTP server port


 
//-----------------------END Variable setup-------------------------------
 
void setup()
{
  pinMode(gateopen, OUTPUT);   // set high to open gate
  pinMode(gateclose, OUTPUT);  // set high to close gate
  pinMode(gatestop, OUTPUT);    // set high to stop gate
  pinMode(gatestatus, INPUT);    //check to see if gate is open. pin high if gate is fully open.
  digitalWrite(gatestatus, HIGH); //Turn on pullup resistor.
  
  Serial.begin(19200);
  Serial.println("Initializing.... ");
// EthernetDHCP.begin(mac);

// we're going to assume that if DHCP comes back with no address, then we'll use the hard-coded one as a backup.
 if (Ethernet.begin(mac) == 0) {
   Serial.println("Failed to configure Ethernet using DHCP");
   Serial.println("Failing to hard set IP");
    Ethernet.begin(mac, ip,gateway, subnet);
   }
  
  //Initialize server
  server.begin();
  //Give server a second to initialize
  delay(1000);
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
}


  
void loop()
{

/* Need to add DHCP Lease management.
Ethernet.maintain(); 
*/

 
//---------------Web Server initialization------------------------------

//see if we have any incoming client requests.

  EthernetClient client = server.available();
  
  if (client) {
    Serial.print("Client Connected at: ");
    time = millis();
    //prints time since program started. Lets print time to serial output at client connect and disconnect
    Serial.println(time);
  
    
    boolean current_line_is_blank = true;   // an http request ends with a blank line
    while (client.connected()) {
      
      /* Need to add some logic to see if a stale session is still open. If so, I want to kill the client/TCP session. Maybe use millis function and compare. Something like below:
       unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > sessionTimeout) { 
     client.stop();
     client.flush();
     previousMillis = 0;
     break; //break out of while loop.
   }
    
    Also, we need to make it easier to specify interval number since;
    A 16MHz oscillator results in the microcontroller having a clock cycle once every 16-millionth of a second. A clock cycle is roughly the time it takes for one instruction cycle (there are exceptions). 
    So on the Arduino, each clock cycle ticks off at every 1/16,000,000 second, which is:

    0.0000000625 seconds
    0.0000625 milliseconds (ms)
    0.0625 microseconds (µs)

    */
      
       if (client.available()) {
        char c = client.read();
          
       if (readString.length() < 100)
      {
        readString += c;
        //Serial.print("readString value: ");
        //Serial.println(readString);
        //As we go through the while look and increment on 'c', readString will get the whole request.
        //example: after going through interation, read string looks like:
        //-----------------------
        //GET /?GATE-CONTROL-IO1=1 HTTP/1.1

        //Host: 10.0.0.5

        //User-Agent: Mozilla/5.0 (X11; Linux i686; rv:16
        // ------------------------
        // I think readString needs to be bigger than 30 char as declared at beginning...check if this is where the memory leak is...
      }       

// OK. we make this very simple. We want to see if we get a specific radio button enabled. We can use the indexOf function to return a value of greater than 0 if it finds a match. 
// if we get a value greater than 0, then we assume we got a hit and then we do what we need to do.
// only issue with this is if more than one radio button is selected.
//Maybe put more logic here in the future to check for this and provide a message back that it is an error.

          if(readString.indexOf("GATE-CONTROL-IO1=1") > 0) {  //Open request
            gatecommand=gateopen;  //use gatecommand like a flag
            //Serial.println("Got Gate Open request. ");
           }
          
          if(readString.indexOf("GATE-CONTROL-IO2=1") > 0) {   //Close request
            gatecommand=gateclose;
            //Serial.println("Got Gate Close Request ");
           }
         
          if(readString.indexOf("GATE-CONTROL-IO3=1") > 0) {   //Stop request
            gatecommand=gatestop;
            //Serial.println("Got Gate Stop Request ");
          } 

           
                
///////////////Finish checking and actions for submit button//////////////////
 
//------------------Standard web Server Jargon-------------------------------
        if (c == 'n' && current_line_is_blank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: html");
          client.println();
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Xboard interface--Gate Control</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("My Gate Interface"); 
          client.println("<br />");
          client.print("//*************************************");
          client.println("<br />");
          client.println("Current IP is: ");
          client.println(Ethernet.localIP());
          client.println("<br />");
          client.print("//*************************************");
          client.println("<br />");
          client.println("<br />");
          client.print("<form>");
          client.print("<input type=radio name=GATE-CONTROL-IO1 value=1 /> Open Gate<br />");
          client.print("<input type=radio name=GATE-CONTROL-IO2 value=1 /> Close Gate<br />");
          client.print("<input type=radio name=GATE-CONTROL-IO3 value=1 /> Stop Gate<br />"); 
          client.print("<input type=submit value=SUBMIT/> </form><br />");
          client.println("</body>");
            break;
            }
 
        if (c == 'n') {
          // we're starting a new line
          current_line_is_blank = true;
        }
        else if (c != 'r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }
    }
//------------------END Standard web Server Jargon-------------------------------
 
//-----------------Print gate status on web page and auto refresh----------------

val = digitalRead(gatestatus);   //READ Gate Status on pin D9
   if (val == HIGH) {
          //printing Gate status. If high then gate is closed based on the contacts I have in place.
         client.print("<font size='5'>Gate status: ");
         client.println("<font color='green' size='5'>CLOSED");
         // OK we want all one string <META HTTP-EQUIV=REFRESH CONTENT=2;url=http://IPADDRESS/>" but I don't know how to embed a variable within the string.
         // so I'm going to break this up into several strings concatenated. use print function to continue printing.
         // use println to add the CR/LF. 
         client.print("<br>Active Pin value: ");
         client.println(gatecommand);
         if (gatecommand > 0) {
            digitalWrite(gatecommand, HIGH);
            delay(2000);
            digitalWrite(gatecommand, LOW);
            gatecommand=0;  //reset to 0 so we don't execute it again.
         }
         //client.print("<META HTTP-EQUIV=REFRESH CONTENT=5;url=http://"); //Autorefresh
         //client.print(Ethernet.localIP());
         //client.println("/>");
                        //Auto-refresh the site after 5 seconds to reset the gate status
         client.print("<br><br>");
         //client.print("\"<a href=\"javascript:document.location.reload();\"");
         //client.print("ONMOUSEOVER=\"window.status='Refresh'; return true\"");
         //client.print("ONMOUSEOUT=\"window.status='ah... that was good'\">");
         //client.print("REFRESH GATE STATUS");
        client.print("<a href=\"http://");
        client.print(Ethernet.localIP());
        client.print("\">REFRESH GATE STATUS");
        client.println("</a>");
         
     }
     
     else{
          client.print("<font size='5'>Gate status: ");
          client.println("<font color='red' size='5'>OPEN");
                   // OK we want all one string <META HTTP-EQUIV=REFRESH CONTENT=2;url=http://IPADDRESS/>" but I don't know how to embed a variable within the string.
         // so I'm going to break this up into several strings concatenated. use print function to continue printing.
         // use println to add the CR/LF. 
         client.print("<br>Active Pin value: ");
         client.println(gatecommand);
        if (gatecommand > 0) {
            digitalWrite(gatecommand, HIGH);
            delay(2000);
            digitalWrite(gatecommand, LOW);
            gatecommand=0;  //reset to 0 so we don't execute it again.
         }    
         //client.print("<META HTTP-EQUIV=REFRESH CONTENT=5;url=http://"); //Autorefresh
         //client.print(Ethernet.localIP());
         //client.println("/>");
         client.print("<br><br>");
         // client.print("\"<a href=\"javascript:document.location.reload();\"");
         //client.print("ONMOUSEOVER=\"window.status='Refresh'; return true\"");
         //client.print("ONMOUSEOUT=\"window.status='ah... that was good'\">");
         //client.print("REFRESH GATE STATUS"); 
        client.print("<a href=\"http://");
        client.print(Ethernet.localIP());
        client.print("\">REFRESH GATE STATUS");
        client.println("</a>");
        }
       
          client.println("<hr />");
          client.println("<hr />");
          client.println("</body></html>");
          //clearing string for next read
          Serial.println("Clearing String for next read");
          readString="";
//-----------------END Print door status on web page and auto refresh----------------
    client.flush();
    Serial.println("Client Flush");
    client.stop();
    Serial.println("Client stop");
    Serial.print("Client Disconnected at: ");
    time = millis();
    //prints time since program started. Lets print time to serial output at client connect and disconnect
    Serial.println(time);
  }
}
 
  
 
 
 //******************************************FUNCTIONS**************************************            
    

 
  
  
