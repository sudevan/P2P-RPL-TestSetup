<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <simulation>
    <title>P2P-RPL v06 large</title>
    <delaytime>0</delaytime>
    <randomseed>123456</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>40.0</transmitting_range>
      <interference_range>80.0</interference_range>
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
        <x>81.60638132921417</x>
        <y>129.99652346297373</y>
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
        <x>106.17180394876435</x>
        <y>50.949639409722344</y>
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
        <x>127.88979821860876</x>
        <y>44.67537219677969</y>
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
        <x>72.1058882544568</x>
        <y>105.10818212072152</y>
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
        <x>118.45812790493001</x>
        <y>88.61912379901209</y>
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
        <x>1.8948735798606842</x>
        <y>147.7507940585003</y>
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
        <x>87.55556477809392</x>
        <y>13.328713330922797</y>
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
        <x>6.723515513676576</x>
        <y>21.073132448196258</y>
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
        <x>36.38055343458657</x>
        <y>77.15254822362894</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-1.794939868458505</x>
        <y>90.55868940826029</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>90.93607851114463</x>
        <y>73.11752990201795</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>11</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>37.573458599648205</x>
        <y>141.3908095586603</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>12</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>59.6305502789489</x>
        <y>67.43202009551744</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>13</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>32.98900117420258</x>
        <y>59.51174051409303</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>14</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>129.77226062554453</x>
        <y>120.29869541673273</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>15</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>133.99378952377762</x>
        <y>68.6978145430918</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>16</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>70.71906785960951</x>
        <y>90.8186001014951</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>17</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>44.75463475349834</x>
        <y>24.290197758946718</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>18</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>23.117733510821314</x>
        <y>115.68917806838304</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>19</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>39.92981718333251</x>
        <y>121.11546181525988</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>20</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>318</width>
    <z>0</z>
    <height>192</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>3.093476173938591 0.0 0.0 3.093476173938591 73.55260371672885 -20.141148027558227</viewport>
    </plugin_config>
    <width>581</width>
    <z>4</z>
    <height>519</height>
    <location_x>302</location_x>
    <location_y>560</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>700</width>
    <z>2</z>
    <height>446</height>
    <location_x>337</location_x>
    <location_y>31</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>7</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>722</width>
    <z>5</z>
    <height>339</height>
    <location_x>1077</location_x>
    <location_y>41</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>2</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>725</width>
    <z>3</z>
    <height>305</height>
    <location_x>868</location_x>
    <location_y>337</location_y>
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
 		if(id == source)
 		{
 		logprint("Failed - p2p route discovery from "+source+" to "+ target);
 		logprint("Starting continue timer");
 		GENERATE_MSG(1000, "continue");
 		state = 0;
 		}
 	}
 	else if( msg.contains("Sending ping"))
 	{
 		if(id == source)
 		{
 		pingsendtime=time;
 		logprint("Ping send time");
 		logprint("Starting ping timeout timer");
 		GENERATE_MSG(1000, "ping timeout");
 		state = 2;
 		packetsendcount++;
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
 		logprint("Paket delay from "+source +" to "+target + " is " +delay);
 		logprint("Starting continue timer");
 		GENERATE_MSG(1000, "continue");
 		state =0;
 		packetrecvcount++;
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
    <z>1</z>
    <height>700</height>
    <location_x>174</location_x>
    <location_y>-15</location_y>
  </plugin>
</simconf>

