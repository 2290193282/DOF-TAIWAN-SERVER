// Migration evidence only.
//
// CItemList's old-server C1/C2, D1/D2, destroy, init and PVF-import bodies
// are canonically provided once by:
//   src/old_data/PVF/data/item_list_pvf_lifecycle.cpp
//   src/old_data/PVF/data/item_list_pvf_import.cpp
//
// This legacy datamanager path intentionally exports no symbol.  Keeping a
// second alias body here would make the same original CItemList identity a
// duplicate provider if the migration-evidence directory were ever compiled.
