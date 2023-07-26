"""
    A python script example to create plot files to build a board:
    Gerber files
    Drill files

    source https://github.com/KiCad/kicad-source-mirror/blob/master/demos/python_scripts_examples/plot_board.py
    additons via https://github.com/beagleboard/pocketbeagle/blob/master/kicad-scripts/kicad-fab.py
"""

import sys, os, subprocess, shutil, platform
from pcbnew import *

if len(sys.argv) < 2:
    print("Usage: python3 script.py [abspath].kicad_pcb")
    sys.exit(1)


# A helper function to convert a UTF8 string for python2 or python3
def fromUTF8Text(afilename):
    if sys.version_info < (3, 0):
        return afilename.encode()
    else:
        return afilename


# Rename the .drl file, so it apears first in the list of gerber files
def get_filename_without_extension(file_path):
    """
    This function returns the filename without extension, regardless of the path.

    :param file_path: The complete file path.
    :return: The filename without extension.
    """
    base_name = os.path.basename(file_path)  # get the filename
    file_name_without_extension = os.path.splitext(base_name)[0]  # remove the extension
    return file_name_without_extension


abspath = sys.argv[1]
if os.path.exists(abspath):
    print("found " + abspath)
    abspath = os.path.abspath(abspath)
    filename = get_filename_without_extension(abspath)
else:
    print("not found " + abspath)
    sys.exit(1)

scriptlocation = os.path.dirname(os.path.realpath(__file__))

board = LoadBoard(abspath)
plotDir = scriptlocation + "/export/" + filename + "_plot/"
print("write to " + plotDir)

pctl = PLOT_CONTROLLER(board)
popt = pctl.GetPlotOptions()
popt.SetOutputDirectory(plotDir)

# Set some important plot options:
# One cannot plot the frame references, because the board does not know
# the frame references.
popt.SetPlotFrameRef(False)
popt.SetSketchPadLineWidth(FromMM(0.1))

popt.SetAutoScale(False)
popt.SetScale(1)
popt.SetMirror(False)
popt.SetUseGerberAttributes(False)  # gerbv doesnt like gerber x2 attributes
# popt.SetIncludeGerberNetlistInfo(True)
# popt.SetUseGerberProtelExtensions(False)
popt.SetUseAuxOrigin(True)

# This by gerbers only (also the name is truly horrid!)
popt.SetSubtractMaskFromSilk(
    False
)  # remove solder mask from silk to be sure there is no silk on pads

# Create a pdf file of the top silk layer
# pctl.SetLayer(F_SilkS)
# pctl.OpenPlotfile("Silk", PLOT_FORMAT_PDF, "Assembly guide")
# pctl.PlotLayer()

# Once the defaults are set it become pretty easy...
# I have a Turing-complete programming language here: I'll use it...
# param 0 is a string added to the file base name to identify the drawing
# param 1 is the layer ID
plot_plan = [
    ("01-Edge_Cuts", Edge_Cuts, "Edges"),
    ("02-SilkTop", F_SilkS, "Silk top"),
    ("03-MaskTop", F_Mask, "Mask top"),
    ("04-CuTop", F_Cu, "Top layer"),
    ("05-SilkBottom", B_SilkS, "Silk top"),
    ("06-MaskBottom", B_Mask, "Mask bottom"),
    ("07-CuBottom", B_Cu, "Bottom layer"),
]

# In Gerber format, Set layer before calling OpenPlotfile is mandatory to generate
# the right Gerber file header.
for layer_info in plot_plan:
    if layer_info[1] <= B_Cu:
        popt.SetSkipPlotNPTH_Pads(True)
    else:
        popt.SetSkipPlotNPTH_Pads(False)

    pctl.SetLayer(layer_info[1])
    pctl.OpenPlotfile(layer_info[0], PLOT_FORMAT_GERBER, layer_info[2])

    print("plot %s" % fromUTF8Text(pctl.GetPlotFileName()))
    pctl.PlotLayer()

# At the end you have to close the last plot, otherwise you don't know when
# the object will be recycled!
pctl.ClosePlot()

# Fabricators need drill files.
# sometimes a drill map file is asked (for verification purpose)
drlwriter = EXCELLON_WRITER(board)
drlwriter.SetMapFileFormat(PLOT_FORMAT_PDF)

mirror = False
minimalHeader = False
offset = VECTOR2I(0, 0)
# False to generate 2 separate drill files (one for plated holes, one for non plated holes)
# True to generate only one drill file
mergeNPTH = True
drlwriter.SetOptions(mirror, minimalHeader, offset, mergeNPTH)

metricFmt = True
drlwriter.SetFormat(metricFmt)

genDrl = True
genMap = True
print("create drill and map files in %s" % fromUTF8Text(pctl.GetPlotDirName()))
drlwriter.CreateDrillandMapFilesSet(pctl.GetPlotDirName(), genDrl, genMap)

# One can create a text file to report drill statistics
rptfn = pctl.GetPlotDirName() + "drill_report.rpt"
drlwriter.GenDrillReportFile(rptfn)

# Rename the .drl file, so it apears first in the list of gerber files
drill_file = pctl.GetPlotDirName() + filename
print("rename " + drill_file + ".drl to " + drill_file + "-00.drl")
#delete previous then rename
if os.path.exists(drill_file + "-00.drl"):
    os.remove(drill_file + "-00.drl")
    os.rename(drill_file + ".drl", drill_file + "-00-drill.drl")

### gerbv preview

# make list of all files in plotdir
plotfiles = []
for file in os.listdir(plotDir):
    # if not pdf or rpt
    if not file.endswith(".pdf") and not file.endswith(".rpt"):
        plotfiles.append(os.path.join(plotDir, file))
plotfiles.sort()

# pass filelist to gerbv, with spaces between filenames
# run only if gerbv is installed
gerbv_executable = "gerbv"
#if windows run using "C:\bin\gerbv\gerbv.exe"
if platform.system() == "Windows":
    gerbv_executable = r"C:\bin\gerbv\gerbv.exe"

if shutil.which(gerbv_executable) is not None:
    subprocess.Popen([gerbv_executable] + plotfiles)
else:
    print(
        f"{gerbv_executable} is not installed or not found on system PATH. consider installing"
    )
