more at [idiot.io](idiot.io)

# candy
Electrodible lolliPCBpops

![iso look](/GFX/asset_iso_v1.1.png?raw=true) 

![basic look](/GFX/asset_basic_2.1.png?raw=true) 

34c3 [video](https://www.youtube.com/watch?v=yhNaNCrcmBk) cooking show and talk  

## tools
using [inkscape](https://inkscape.org) with [svg2shenzen](https://github.com/badgeek/svg2shenzhen) extension - a wrapper/fork to [Kicad's](https://kicad.github.io) own bitmap2component plugin.  
basicly - design a board in SVG vector format and export it as gerber files, ready for fabrication.  

included in the repo is a script to automate the gerber file generation.  
so after you run `extensins>svg2shenzhen>export`  
run  
```python2 ../kicad_gerber_gen.py IRiotCandy-svg2shnzn.kicad_pcb```

[gerbv](http://gerbv.geda-project.org) was then used to preview the gerber output and move the silkscreen to its right position. if you have it in your path, the previous python script should auto open the generate dgerber files fo you. 
