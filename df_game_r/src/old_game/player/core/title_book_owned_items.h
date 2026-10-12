#ifndef OLDSRV_TITLE_BOOK_OWNED_ITEMS_H_
#define OLDSRV_TITLE_BOOK_OWNED_ITEMS_H_
#include <stdint.h>
class CTitleBook;
namespace title_book {
// Read actual entries, including unrestricted slots; never modifies ownership.
bool ContainsOwnedTitle(CTitleBook* book, uint32_t item_id);
}
#endif
