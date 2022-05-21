Welcome to pycsmm's documentation!
================================

pycsmm is the module for CSMM's Python API. Mod files should be .py scripts with an instance of
CSMMMod assigned to a variable (at module scope) named "mod". For the mod to perform
actual functionality onto Fortune Street, the "mod" variable instance should inherit
one of the mod interface classes such as GeneralInterface, ArcFileInterface, etc.

The CSMM mod loader works with folders called "mod packs", which contain a modlist.txt specifying
which mods to load as well as the user-created mod files themselves. To export the default
modlist.txt, go to Tools > Export default modlist.txt in CSMM.

Example usage\:

.. literalinclude:: testmod.py
	:language: python

.. toctree::
	:maxdepth: 2
	:caption: Contents:

CSMM Reference
==============

.. automodule:: pycsmm
	:imported-members:
	:members:
	:undoc-members:
	:special-members: __init__, __getitem__
	:inherited-members:

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
