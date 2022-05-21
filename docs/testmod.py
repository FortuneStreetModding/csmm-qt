import pycsmm
import sys

class Mod(pycsmm.CSMMMod, pycsmm.GeneralInterface, pycsmm.ArcFileInterface):
	def __init__(self):
		pycsmm.CSMMMod.__init__(self)
		pycsmm.GeneralInterface.__init__(self)
		pycsmm.ArcFileInterface.__init__(self)
	def modId(self):
		return "testmod"
	def loadFiles(self, root, gameInstance, modList):
		print('modpack directory:', self.modpackDir(), file=sys.stderr)
		print('number of mods:', len(modList), file=sys.stderr)
		for mod in modList:
			print(mod.modId(), type(mod), file=sys.stderr)
	def saveFiles(self, root, gameInstance, modList):
		print(len(modList), file=sys.stderr)
		for mod in modList:
			print(mod.modId(), type(mod), file=sys.stderr)
		for descriptor in gameInstance.mapDescriptors:
			print(descriptor.targetAmount, file=sys.stderr)
	def modifyArcFile(self):
		return {
			"files/game/game_sequence.arc": lambda root, gameInstance, modList, extractedPath: print("extracted path in testmod:", extractedPath, file=sys.stderr)
		}

mod = Mod()
