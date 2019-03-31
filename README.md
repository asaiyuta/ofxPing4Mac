# ofxPing4Mac
 send a ping with mac to check the network on openframeworks
 
## usage
~~~
ofxPing4Mac pingChecker;
//ofxPing4MacMulti<3> pingChecker; //process in parallel


void setup(){
  pingChecker.setWaitTime(10) // on msec default 50
  pingChecker.add("you check ip1");
  pingChecker.add("you check ip2");
}

void update(){
  if(pingChecker.getState("you check ip1")){
    ofLogError() << "ip1";
  }
  
  pingChecker.add("you check ip3"); 
  pingChecker.remove("you check ip2"); //you can add and delete at any time

}

void draw(){
   ofDrawBitmapStringHighlight(pingChecker.getAllStateStr(), 10, 15);
}

~~~


