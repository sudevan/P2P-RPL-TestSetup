<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <simulation>
    <title>p2p-etx-5-nodes</title>
    <delaytime>0</delaytime>
    <randomseed>123456</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>common mote</description>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/ipv6/rpl-p2p/p2p-sim.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>48.25298585775021</x>
        <y>23.88954163777558</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>85.8178379264408</x>
        <y>20.18690534234007</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>60.47458098412745</x>
        <y>38.83131640074971</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>37.34048702232399</x>
        <y>69.00806724577686</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>18.08707106040525</x>
        <y>41.846998299498644</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>318</width>
    <z>5</z>
    <height>192</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <viewport>2.9085747750301665 0.0 0.0 2.9085747750301665 13.392401359527161 -25.715123664652033</viewport>
    </plugin_config>
    <width>300</width>
    <z>2</z>
    <height>300</height>
    <location_x>757</location_x>
    <location_y>-6</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>1294</width>
    <z>4</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>398</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <mote>2</mote>
      <mote>3</mote>
      <mote>4</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <split>125</split>
      <zoomfactor>500.0</zoomfactor>
    </plugin_config>
    <width>1294</width>
    <z>3</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>548</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 */


//TIMEOUT(1000000000);
function logprint(str)
{
	log.log(time+" : "+id+" : "+str+"\n");
}

state=0;
packetsendcount=0;
packetrecvcount=0;
totaldelaytime=0;
etxbeforep2p=0;
etxafterp2p=0;
countbeforep2p=0;
countaftep2p=0;
timoutetx=30;
allm = sim.getMotes();
num_of_nodes = allm.length;
count = 0;
while (count++ &lt;   num_of_nodes ) {
  YIELD_THEN_WAIT_UNTIL(msg.contains("Ready"));
  log.log(count + ": Mote  OK\n");
}

for(var i = 0; i &lt; allm.length; i++)
{
	write(allm[i],"n "+ num_of_nodes.toString() ); 
}
/* Wait 3 seconds (4000ms) */
log.log("Waiting for 30 seconds\n");
GENERATE_MSG(30000, "continue");

/*state - 1 p2p route discovery initiated
state 2 -&gt; ping sent
*/
while(true)
{
	 try{

 		YIELD();
 	} catch (e) {
 		//Close files.

 		//Rethrow exception again, to end the script.
 		pdr = packetrecvcount/packetsendcount;
 		avgdelay = totaldelaytime/packetrecvcount;
 		avgafter = etxafterp2p/countaftep2p;
 		avgextbefore = etxbeforep2p/countbeforep2p;
 		log.log("Result: Point to point " +num_of_nodes.toString() + "nodes \n");
 		log.log("Average etx after p2p route "+num_of_nodes.toString() + "nodes : "+ avgafter.toString()+"\n");
 		log.log("Average etx before p2p route : "+ num_of_nodes.toString() + "nodes : "+ avgextbefore.toString()+"\n");
 		log.testOK();
 		throw('test script killed');
 	}
	if( msg.equals("continue"))
	{
		do
		{
			source = Math.floor((Math.random() * num_of_nodes) + 1);
			target = Math.floor((Math.random() * num_of_nodes) + 1);
		}while(source == target);
		logprint("Initiated etx measurement from "+source+" to "+ target);
		write(sim.getMoteWithID(source), "m "+target.toString());
		state=3;
 	}
 	else if (msg.contains("ETX for route") )
 	{
 		if(id == source)
 		{
 			a1 = msg.split("-");
			etx=parseFloat(a1[1]);
	 		logprint(state.toString() + " Success ETX Measurement");
	 		logprint(msg);
	 		GENERATE_MSG(1000, "continue");
	 		 			
 			if(state == 3 )
 			{
 				etxbeforep2p = etxbeforep2p +etx;
 				countbeforep2p++;
 				logprint("etx before "+etxbeforep2p);
 			}
 			else if( state == 5 )
 			{
 			 	etxafterp2p = etxafterp2p +etx;
 				countaftep2p++;
 				logprint("etx after "+etxafterp2p);	
 			}
 			state = 0;
	 	}
 	}
 	else if (msg.contains("Received DRO - from app") )
 	{
 		logprint("Success - p2p route discoverd to from "+source+" to "+ target);
 		if ( state == 4 )
 		{
	 		logprint("Initiated etx measurement from "+source+" to "+ target);
			write(sim.getMoteWithID(source), "m "+target.toString());
			state=5;
		}
 	}
 	else if( msg.contains("Last Operation timed out"))
 	{
 		if(id == source)
 		{
 			if( state == 2)
 			{
		 		logprint("Failed - p2p route discovery from "+source+" to "+ target);
		 		logprint("Starting continue timer");
		 		GENERATE_MSG(1000, "continue");
		 		state = 0;
	 		}
	 		else  if ( state == 3)
 			{
	 			logprint("ETX measure time out from "+source +" to "+target);
	 			logprint("Initiated p2p route discovery from "+source+" to "+ target);
				write(sim.getMoteWithID(source), "r "+target.toString());
				state=4;
				etxbeforep2p = etxbeforep2p +timoutetx;
 				countbeforep2p++;
 			}
 			else if(state ==5)
 			{
 				logprint("ETX measure after p2p route time out from "+source +" to "+target);
	 			GENERATE_MSG(1000, "continue");
	 			state = 0;
	 			etxafterp2p = etxafterp2p +timoutetx;
 				countaftep2p++;
 			}
 		}
 		
 	}
 	else if( msg.contains("Sending ping"))
 	{
 		if(id == source)
 		{
	 		pingsendtime=time;
	 		packetsendcount++;
	 		logprint("Ping send time");
	 		logprint("Ping send count : "+packetsendcount);
	 		logprint("Starting ping timeout timer");
	 		GENERATE_MSG(1000, "ping timeout");
	 		state = 2;
 		
 		}
 	}
 	else if( msg.contains("Received ping"))
 	{
 		if(id == target)
 		{
 		pingrecvtime = time;
 		delay = pingrecvtime - pingsendtime;
 		totaldelaytime =totaldelaytime+delay;
 		logprint(msg);
 		
 		packetrecvcount++;
 		logprint("Receive Count : "+packetrecvcount);
 		logprint("Paket delay from "+source +" to "+target + " is " +delay);
 		logprint("Starting continue timer");
 		GENERATE_MSG(1000, "continue");
 		state =0;
 		
 		}
 	}
 	else if( msg.contains("ping timeout"))
 	{	
 		if ( state == 2)
 		{
 			logprint("Ping time out from "+source +" to "+target);
 			GENERATE_MSG(1000, "continue");
 			state = 0;
 			
 		}

 	}
	else
	{
			//log.log( id+ " : " + msg + "\n");
	}
}
log.testOK();</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>698</height>
    <location_x>116</location_x>
    <location_y>8</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
      <analyzers name="6lowpan-pcap" />
    </plugin_config>
    <width>500</width>
    <z>1</z>
    <height>300</height>
    <location_x>150</location_x>
    <location_y>150</location_y>
  </plugin>
</simconf>

