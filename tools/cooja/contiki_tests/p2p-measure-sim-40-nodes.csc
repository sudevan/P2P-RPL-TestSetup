<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <simulation>
    <title>p2petx-40-nodes</title>
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
      <description>common node</description>
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
        <x>114.02386431900835</x>
        <y>83.22403188524959</y>
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
        <x>26.521488859949148</x>
        <y>98.31802794553167</y>
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
        <x>30.683995330325352</x>
        <y>73.33035243338382</y>
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
        <x>13.44441319416626</x>
        <y>41.658262448658604</y>
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
        <x>63.05721301050502</x>
        <y>85.91651602339803</y>
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
        <x>48.999086083021545</x>
        <y>32.27550218058729</y>
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
        <x>52.492365366556534</x>
        <y>9.31192611119538</y>
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
        <x>48.74501271629287</x>
        <y>82.61507015903027</y>
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
        <x>31.561631797676416</x>
        <y>50.324430517299604</y>
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
        <x>12.699414071363602</x>
        <y>38.93620239394513</y>
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
        <x>84.25916276149376</x>
        <y>52.384846611179306</y>
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
        <x>36.57192276981191</x>
        <y>63.368818072135</y>
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
        <x>7.010299825912137</x>
        <y>93.89326798805038</y>
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
        <x>0.4950244027285833</x>
        <y>39.01717953430688</y>
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
        <x>23.478246869732</x>
        <y>78.65830098045257</y>
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
        <x>88.54841573490127</x>
        <y>87.71592277497314</y>
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
        <x>63.43065573088139</x>
        <y>17.899718187172798</y>
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
        <x>83.79274021446243</x>
        <y>17.1820762919224</y>
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
        <x>76.78867023329843</x>
        <y>95.34931459528254</y>
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
        <x>52.03902417111289</x>
        <y>95.63799354234683</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>20</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>12.686454111700662</x>
        <y>57.197234739226076</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>21</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>48.67267311392989</x>
        <y>63.773949447113786</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>22</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>21.5022844901121</x>
        <y>43.10497685275959</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>23</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>55.45115703115929</x>
        <y>73.38280132753886</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>24</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>42.902275238600616</x>
        <y>12.792494517725949</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>25</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>25.665413537040337</x>
        <y>12.276846022990373</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>26</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>93.42194377554111</x>
        <y>62.79356218782459</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>27</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>7.19779091968965</x>
        <y>70.27104514564567</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>28</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>17.393422356791667</x>
        <y>31.2866584045694</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>29</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>43.06179704845614</x>
        <y>91.10663470164143</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>30</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>80.43947689297516</x>
        <y>65.3112342929574</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>31</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>60.64726238003034</x>
        <y>71.15163644412046</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>32</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>66.5688778361337</x>
        <y>57.47232361283552</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>33</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>45.23170718553583</x>
        <y>24.44483358442896</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>34</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>50.32829409793821</x>
        <y>30.15815780396416</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>35</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>14.905751516912968</x>
        <y>62.97901241021453</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>36</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>2.6382060338843605</x>
        <y>59.521568480267184</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>37</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>35.885352484904956</x>
        <y>4.817437862308916</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>38</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>11.946365999860065</x>
        <y>42.61984040539174</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>39</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>25.672581925636962</x>
        <y>45.41176885649597</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>40</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>318</width>
    <z>3</z>
    <height>192</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <viewport>3.052970523506524 0.0 0.0 3.052970523506524 54.188604173906704 -0.4347685197261419</viewport>
    </plugin_config>
    <width>468</width>
    <z>1</z>
    <height>371</height>
    <location_x>774</location_x>
    <location_y>11</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>1294</width>
    <z>5</z>
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
      <mote>5</mote>
      <mote>6</mote>
      <mote>7</mote>
      <mote>8</mote>
      <mote>9</mote>
      <mote>10</mote>
      <mote>11</mote>
      <mote>12</mote>
      <mote>13</mote>
      <mote>14</mote>
      <mote>15</mote>
      <mote>16</mote>
      <mote>17</mote>
      <mote>18</mote>
      <mote>19</mote>
      <mote>20</mote>
      <mote>21</mote>
      <mote>22</mote>
      <mote>23</mote>
      <mote>24</mote>
      <mote>25</mote>
      <mote>26</mote>
      <mote>27</mote>
      <mote>28</mote>
      <mote>29</mote>
      <mote>30</mote>
      <mote>31</mote>
      <mote>32</mote>
      <mote>33</mote>
      <mote>34</mote>
      <mote>35</mote>
      <mote>36</mote>
      <mote>37</mote>
      <mote>38</mote>
      <mote>39</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <split>125</split>
      <zoomfactor>500.0</zoomfactor>
    </plugin_config>
    <width>1294</width>
    <z>4</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>548</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
      <analyzers name="6lowpan-pcap" />
    </plugin_config>
    <width>500</width>
    <z>2</z>
    <height>300</height>
    <location_x>120</location_x>
    <location_y>120</location_y>
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
    <location_x>97</location_x>
    <location_y>5</location_y>
  </plugin>
</simconf>

