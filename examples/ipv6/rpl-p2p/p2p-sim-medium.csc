<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <simulation>
    <title>P2P-RPL v06 medium</title>
    <delaytime>-13</delaytime>
    <randomseed>123456</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGMConstantLoss
      <transmitting_range>50.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #sky1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/ipv6/rpl-p2p/p2p-sim.c</source>
      <commands EXPORT="discard">make p2p-sim.sky TARGET=sky</commands>
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
        <x>111.59925018054851</x>
        <y>11.598708472266114</y>
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
        <x>72.75550414912952</x>
        <y>17.330279061502235</y>
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
        <x>99.96538136750321</x>
        <y>44.76137479384649</y>
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
        <x>157.7034135138084</x>
        <y>18.878808659295885</y>
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
        <x>69.93798815025724</x>
        <y>57.526074589392294</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>106.7919694895167</x>
        <y>76.33964548286043</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>140.29558888884353</x>
        <y>59.33011563397143</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>151.3775553055439</x>
        <y>65.51539921538561</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>8</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>192.01474205777868</x>
        <y>45.29478832305462</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>318</width>
    <z>1</z>
    <height>192</height>
    <location_x>10</location_x>
    <location_y>3</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <viewport>4.2521683488090085 0.0 0.0 4.2521683488090085 -271.9335541373579 45.03565733282371</viewport>
    </plugin_config>
    <width>581</width>
    <z>0</z>
    <height>521</height>
    <location_x>512</location_x>
    <location_y>-6</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>620</width>
    <z>7</z>
    <height>384</height>
    <location_x>8</location_x>
    <location_y>196</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>2</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>597</width>
    <z>6</z>
    <height>409</height>
    <location_x>10</location_x>
    <location_y>581</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>3</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>692</width>
    <z>4</z>
    <height>519</height>
    <location_x>1223</location_x>
    <location_y>5</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>5</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>580</width>
    <z>5</z>
    <height>468</height>
    <location_x>634</location_x>
    <location_y>528</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>8</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>697</width>
    <z>3</z>
    <height>468</height>
    <location_x>1220</location_x>
    <location_y>529</location_y>
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
log.log("Waiting for 3 seconds\n");
GENERATE_MSG(3000, "continue");

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
 		log.log("Packet deleivery ratio : "+ pdr.toString()+"\n");
 		log.log("Average packet delay : "+ avgdelay.toString()+"\n");
 		throw('test script killed');
 	}
	if( msg.equals("continue"))
	{
		do
		{
			source = Math.floor((Math.random() * num_of_nodes) + 1);
			target = Math.floor((Math.random() * num_of_nodes) + 1);
		}while(source == target);
		logprint("Initiated p2p route discovery from "+source+" to "+ target);
		write(sim.getMoteWithID(source), "r "+target.toString());
		state=1;
 	}
 	else if (msg.contains("Received DRO - from app") )
 	{
 		logprint("Success - p2p route discoverd to from "+source+" to "+ target);
 		logprint("Initiated - ping  from "+source+" to "+ target);
 		write(sim.getMoteWithID(source), "p "+target.toString());

 	}
 	else if( msg.contains("Last Operation timed out"))
 	{
 		logprint("Failed - p2p route discovery from "+source+" to "+ target);
 		logprint("Starting continue timer");
 		GENERATE_MSG(1000, "continue");
 		state = 0;
 	}
 	else if( msg.contains("Sending ping"))
 	{
 		pingsendtime=time;
 		logprint("Ping send time");
 		logprint("Starting ping timeout timer");
 		GENERATE_MSG(1000, "ping timeout");
 		state = 2;
 		packetsendcount++;
 	}
 	else if( msg.contains("Received ping"))
 	{
 		pingrecvtime = time;
 		delay = pingrecvtime - pingsendtime;
 		totaldelaytime =totaldelaytime+delay;
 		logprint(msg);
 		logprint("Paket delay from "+source +" to "+target + " is " +delay);
 		logprint("Starting continue timer");
 		GENERATE_MSG(1000, "continue");
 		state =0;
 		packetrecvcount++;
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
    <width>567</width>
    <z>2</z>
    <height>632</height>
    <location_x>164</location_x>
    <location_y>-23</location_y>
  </plugin>
</simconf>

