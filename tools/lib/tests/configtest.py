#!/usr/bin/python

#
# Config file test app (together with test.cfg file)
#

import os, sys

sys.path.append("..")

import configfile

cfg = configfile.ConfigFile("test.cfg")

cfg.setCfgValue("name1", "value1")
cfg.setCfgValue("name2", "value2")
cfg.selectSection("user")
cfg.setCfgValue("username", "janis")
cfg.setCfgValue("acceptable_names", ["john", "janis"])
cfg.load()

print cfg.cfg.options("main")
print cfg.cfg.options("user")
print cfg.getCfgValue("username")
print type(cfg.getCfgValue("username"))
print cfg.getCfgValueAsList("acceptable_names")
print cfg.getCfgValueAsList("list_in_list")
cfg.selectSection("main")
print cfg.getCfgValueAsInt("a_number")
print type(cfg.getCfgValueAsInt("a_number"))
print cfg.getCfgValueAsBool("a_bool")
print type(cfg.getCfgValueAsBool("a_bool"))

cfg.filename = "test-mod.cfg"
cfg.selectSection("main")
cfg.setCfgValue("name1", "value1mod2")
cfg.setCfgValue("a_number", 14)
cfg.selectSection("user")
cfg.setCfgValue("acceptable_names", ["john", "janis", "ivan"])
cfg.setCfgValue("list_in_list2", ["[baz]", "[foo, bar]"])
cfg.setCfgValue("list_in_list3", ["first", "[second-one, second-third]"])
cfg.save()
