solohm
======

Open Source Solar Panel Electronic Load

This project is a hardware/firmware design to aid in testing of solar panels. solohm is powered from the panel's output and an off-the-shelf lipo battery, it can load the panel at its maximum power point, WiFi UDP broadcast current and voltage readings packets, perform IV curve sweeps and measure many temperature of many panel locations. 

SolOhm replaces bulky expensive, AC-powered, non-waterproof, non-high-temperature rated, multi-unit, wired-communication test equipment with a single unit that's inexpensive, panel-powered, waterproof, industrial temperature rated, and wireless. The goal is 5 minute install: bolt SolOhm to your frame, connect your panel's + and - connectors, attach temperature probes and start testing.  

Specifications
<ul>
<li>Input voltage - 50VDC maximum</li>
<li>Input current - 10A maximum, fused</li>
<li>Input power - 300W maximum</li>
<li>Operating temperature - &lt; 130F, acquisition electronics TEC cooled</li>
<li>Display - OLED graphic display</li>
<li>User interface - IR remote DirectTV RC65X or Sony TV</li> 
<li>MPPT tracking - periodic sweep, perturb/observe, manual</li>
<li>Mechanical - frame/poll attachment, IP63, extruded aluminium, glass/plexiglass, passive heatsink</li>
<li>IV sweep time - &lt; 1 sec, VOC to ISC, 500 samples, Vpanel@ISC < 0.25V@8A<li>
<li>ADC resolution - 12 bit, 4096 steps full scale</li>
<li>Temperature probes - DS18B20, COTS stainless steel, multi-drop, +/-0.5C over -10C to +85C</li>
<li>Power supply - COTS 3S LIPO battery, panel powered charger, 24 hour operation</li>
<li>Data transmission - UDP broadcast and control, HTTP REST</li>
<li>Data collection - micro SD slot</li>
<li>Timestamp - UTC via NTP source with battery-backed real time clock</li>
<li>Firmware - OTA updates (planned)</li>
<li>Calibration - Requires 40V@10A power supply, precision voltmeter and ammeter</li>



</ul>
