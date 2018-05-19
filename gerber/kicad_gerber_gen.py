'''
    A python script example to create plot files to build a board:
    Gerber files
    Drill files

    source https://github.com/KiCad/kicad-source-mirror/blob/master/demos/python_scripts_examples/plot_board.py
'''

import sys

from pcbnew import *
filename=sys.argv[1]

board = LoadBoard(filename)

plotDir = "plot/"

pctl = PLOT_CONTROLLER(board)

popt = pctl.GetPlotOptions()

popt.SetOutputDirectory(plotDir)

# Set some important plot options:
popt.SetPlotFrameRef(False)     #do not change it
popt.SetLineWidth(FromMM(0.15)) # default line width to plot items having no line thickiness defined

popt.SetAutoScale(False)        #do not change it
popt.SetScale(1)                #do not change it
popt.SetMirror(False)
popt.SetUseGerberAttributes(False) #true will set it to gerber x2, manyboard fab houses dont like this
popt.SetUseGerberProtelExtensions(True) # change extension from default .gbr
popt.SetExcludeEdgeLayer(True); # true will include edge in copper, not good
popt.SetScale(1)
popt.SetUseAuxOrigin(True)

# This by gerbers only
popt.SetSubtractMaskFromSilk(False)
# Disable plot pad holes
popt.SetDrillMarksType( PCB_PLOT_PARAMS.NO_DRILL_SHAPE );
# Skip plot pad NPTH when possible: when drill size and shape == pad size and shape
# usually sel to True for copper layers
popt.SetSkipPlotNPTH_Pads( False );

# param 0 is the layer ID
# param 1 is a string added to the file base name to identify the drawing
# param 2 is a comment
# Create filenames in a way that if they are sorted alphabetically, they
# are shown in exactly the layering the board would look like. So
#   gerbv *
# just makes sense.


plot_plan = [
    ( Edge_Cuts, "0-Edge_Cuts",   "Edges" ),

    ( F_Paste,   "1-PasteTop",    "Paste top" ),
    ( F_SilkS,   "2-SilkTop",     "Silk top" ),
    ( F_Mask,    "3-MaskTop",     "Mask top" ),
    ( F_Cu,      "4-CuTop",       "Top layer" ),

    ( B_Cu,      "5-CuBottom",    "Bottom layer" ),
    ( B_Mask,    "6-MaskBottom",  "Mask bottom" ),
    ( B_SilkS,   "7-SilkBottom",  "Silk top" ),
    ( B_Paste,   "8-PasteBottom", "Paste Bottom" ),
]


for layer_info in plot_plan:
    if layer_info[1] <= B_Cu:
        popt.SetSkipPlotNPTH_Pads( True )
    else:
        popt.SetSkipPlotNPTH_Pads( False )

    pctl.SetLayer(layer_info[0])
    pctl.OpenPlotfile(layer_info[1], PLOT_FORMAT_GERBER, layer_info[2])
    print 'plot %s' % pctl.GetPlotFileName()
    if pctl.PlotLayer() == False:
        print "plot error"

# At the end you have to close the last plot, otherwise you don't know when
# the object will be recycled!
pctl.ClosePlot()

# Fabricators need drill files.
# sometimes a drill map file is asked (for verification purpose)
drlwriter = EXCELLON_WRITER( board )
drlwriter.SetMapFileFormat( PLOT_FORMAT_PDF )

mirror = False
minimalHeader = False
offset = wxPoint(0,0)
# False to generate 2 separate drill files (one for plated holes, one for non plated holes)
# True to generate only one drill file
mergeNPTH = True
drlwriter.SetOptions( mirror, minimalHeader, offset, mergeNPTH )

metricFmt = True
drlwriter.SetFormat( metricFmt )

genDrl = True
genMap = True
print 'create drill and map files in %s' % pctl.GetPlotDirName()
drlwriter.CreateDrillandMapFilesSet( pctl.GetPlotDirName(), genDrl, genMap );

