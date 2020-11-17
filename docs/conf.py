# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
# sys.path.insert(0, os.path.abspath('.'))

sys.path.insert(1, os.path.abspath('/Users/gregor/Applications/doxyrest-2.1.0-mac/share/doxyrest/sphinx'))
sys.path.append("/Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/site-packages/breathe")

# -- Project information -----------------------------------------------------

project = 'CommandStation EX NetworkInterface'
copyright = '2020, Gregor Baues'
author = 'Gregor Baues'

breathe_projects = { "cs-ex-nwi": "/Users/gregor/Documents/PlatformIO/Projects/EthernetInterface/docs" }
breathe_default_project = "cs-ex-nwi"

# The full version, including alpha/beta/rc tags
release = '0.9'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.pngmath', 
    'sphinx.ext.todo', 
    'breathe'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'page_index.rst']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']