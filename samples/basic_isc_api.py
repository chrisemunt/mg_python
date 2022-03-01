#
#   mg_python Test Page
#
#      Copyright (c) 2008-2022 M/Gateway Developments Ltd.
#      All rights reserved.
#
#   This test page writes to global ^MyGlobal
#

import mg_python

print("\nAccess to InterSystems database via the API")

# Modify the following parameters for your installation
dbtype = "Cache"
path = "/cache20181/mgr"
username = "_SYSTEM"
password = "SYS"
envvars = ""
params = ""
db = 0

mg_python.m_set_uci(db, "USER")
mg_python.m_bind_server_api(db, dbtype, path, username, password, envvars, params)

print("\nmg_python version: ", mg_python.m_ext_version())

print('\nSet up some records in ^MyGlobal ...')
for key in range(db, 10):
   mg_python.m_set(db, "^MyGlobal", key, "Record #" + str(key))

print('\nParse records in order ($Order) ...')
key1 = mg_python.m_order(db, "^MyGlobal", "")
while (key1 != ""):
   print(key1, " = ", mg_python.m_get(db, "^MyGlobal", key1))
   key1 = mg_python.m_order(db, "^MyGlobal", key1)

print('\nRecord #5 defined: ' + mg_python.m_defined(db, "^MyGlobal", 5))
print('Delete Record #5: ' + mg_python.m_kill(db, "^MyGlobal", 5))
print('Record #5 defined: ' + mg_python.m_defined(db, "^MyGlobal", 5))

print('\nParse records in reverse order (reverse $Order) ...')
key1 = mg_python.m_previous(db, "^MyGlobal", "")
while (key1 != ""):
   print(key1, " = ", mg_python.m_get(db, "^MyGlobal", key1))
   key1 = mg_python.m_previous(db, "^MyGlobal", key1)

# print('\nInvoke M function: $$add^math(2,3): ' + mg_python.m_function(db, "add^math", 2, 3))

print('\nInvoke an InterSystems class method: %Library.Date.DisplayToLogical(10/10/2019): ' + mg_python.m_classmethod(db, "%Library.Date", "DisplayToLogical", "10/10/2019"))

print('\nError messages:', mg_python.m_get_last_error(0))

mg_python.m_release_server_api(db)
