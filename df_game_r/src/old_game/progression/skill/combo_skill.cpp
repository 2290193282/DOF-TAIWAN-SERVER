#include "old_game/progression/skill/combo_skill.h"

#include "common/config.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/data/data_manager.h"

#include <string.h>
#include <utility>
#include <vector>


namespace {

int ComboTreeIndex(::ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_0) {
        return 0;
    }
    if (kind == ENUM_SKILL_TREE_1) {
        return 1;
    }
    return -1;
}

void FillSixCommands(std::vector<int>& out, const uint16_t* values) {
    out.clear();
    out.reserve(6);
    for (int index = 0; index <= 5; ++index) {
        out.push_back(values[index]);
    }
}

void CopyLimitedCommands(std::vector<int>& out,
                         const std::vector<int>& in) {
    out.assign(6, 0);
    const unsigned int count = in.size() < 6u ? in.size() : 6u;
    for (unsigned int index = 0; index < count; ++index) {
        out[index] = in[index];
    }
}

}  // namespace

ComboSkillRbNode::ComboSkillRbNode(const ComboSkillMapValue& source)
    : value(source) {
    color = ComboSkillRbNodeBase::kRed;
    parent = 0;
    left = 0;
    right = 0;
}

ComboSkillMap::iterator::iterator()
    : node_(0) {}

ComboSkillMap::iterator::iterator(ComboSkillRbNodeBase* node)
    : node_(node) {}

ComboSkillMap::value_type& ComboSkillMap::iterator::operator*() const {
    return ComboSkillMap::as_node(node_)->value;
}

ComboSkillMap::value_type* ComboSkillMap::iterator::operator->() const {
    return &ComboSkillMap::as_node(node_)->value;
}

ComboSkillMap::iterator& ComboSkillMap::iterator::operator++() {
    node_ = ComboSkillMap::increment(node_);
    return *this;
}

bool ComboSkillMap::iterator::operator==(const iterator& other) const {
    return node_ == other.node_;
}

bool ComboSkillMap::iterator::operator!=(const iterator& other) const {
    return node_ != other.node_;
}

ComboSkillMap::const_iterator::const_iterator()
    : node_(0) {}

ComboSkillMap::const_iterator::const_iterator(
    const ComboSkillRbNodeBase* node)
    : node_(node) {}

ComboSkillMap::const_iterator::const_iterator(const iterator& other)
    : node_(other.node_) {}

const ComboSkillMap::value_type&
ComboSkillMap::const_iterator::operator*() const {
    return ComboSkillMap::as_node(node_)->value;
}

const ComboSkillMap::value_type*
ComboSkillMap::const_iterator::operator->() const {
    return &ComboSkillMap::as_node(node_)->value;
}

ComboSkillMap::const_iterator& ComboSkillMap::const_iterator::operator++() {
    node_ = ComboSkillMap::increment(node_);
    return *this;
}

bool ComboSkillMap::const_iterator::operator==(
    const const_iterator& other) const {
    return node_ == other.node_;
}

bool ComboSkillMap::const_iterator::operator!=(
    const const_iterator& other) const {
    return node_ != other.node_;
}

ComboSkillMap::ComboSkillMap()
    : state_() {
    state_.header.color = ComboSkillRbNodeBase::kRed;
    state_.header.parent = 0;
    state_.header.left = &state_.header;
    state_.header.right = &state_.header;
    state_.node_count = 0;
}

ComboSkillMap::ComboSkillMap(const ComboSkillMap& other)
    : state_() {
    state_.header.color = ComboSkillRbNodeBase::kRed;
    state_.header.parent = 0;
    state_.header.left = &state_.header;
    state_.header.right = &state_.header;
    state_.node_count = 0;
    for (const_iterator it = other.begin(); it != other.end(); ++it) {
        insert(*it);
    }
}

ComboSkillMap::~ComboSkillMap() {
    clear();
}

ComboSkillMap& ComboSkillMap::operator=(const ComboSkillMap& other) {
    if (this != &other) {
        clear();
        for (const_iterator it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
    }
    return *this;
}

ComboSkillRbNodeBase* ComboSkillMap::minimum(
    ComboSkillRbNodeBase* node) {
    while (node->left) {
        node = node->left;
    }
    return node;
}

const ComboSkillRbNodeBase* ComboSkillMap::minimum(
    const ComboSkillRbNodeBase* node) {
    while (node->left) {
        node = node->left;
    }
    return node;
}

ComboSkillRbNodeBase* ComboSkillMap::increment(
    ComboSkillRbNodeBase* node) {
    if (node->right) {
        return minimum(node->right);
    }
    ComboSkillRbNodeBase* parent = node->parent;
    while (node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    if (node->right != parent) {
        node = parent;
    }
    return node;
}

const ComboSkillRbNodeBase* ComboSkillMap::increment(
    const ComboSkillRbNodeBase* node) {
    if (node->right) {
        return minimum(node->right);
    }
    const ComboSkillRbNodeBase* parent = node->parent;
    while (node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    if (node->right != parent) {
        node = parent;
    }
    return node;
}

ComboSkillRbNode* ComboSkillMap::as_node(ComboSkillRbNodeBase* node) {
    return static_cast<ComboSkillRbNode*>(node);
}

const ComboSkillRbNode* ComboSkillMap::as_node(
    const ComboSkillRbNodeBase* node) {
    return static_cast<const ComboSkillRbNode*>(node);
}

ComboSkillRbNodeBase*& ComboSkillMap::root() {
    return state_.header.parent;
}

ComboSkillRbNodeBase* ComboSkillMap::root() const {
    return state_.header.parent;
}

void ComboSkillMap::rotate_left(ComboSkillRbNodeBase* node) {
    ComboSkillRbNodeBase* pivot = node->right;
    node->right = pivot->left;
    if (pivot->left) {
        pivot->left->parent = node;
    }
    pivot->parent = node->parent;
    if (node == root()) {
        root() = pivot;
    } else if (node == node->parent->left) {
        node->parent->left = pivot;
    } else {
        node->parent->right = pivot;
    }
    pivot->left = node;
    node->parent = pivot;
}

void ComboSkillMap::rotate_right(ComboSkillRbNodeBase* node) {
    ComboSkillRbNodeBase* pivot = node->left;
    node->left = pivot->right;
    if (pivot->right) {
        pivot->right->parent = node;
    }
    pivot->parent = node->parent;
    if (node == root()) {
        root() = pivot;
    } else if (node == node->parent->right) {
        node->parent->right = pivot;
    } else {
        node->parent->left = pivot;
    }
    pivot->right = node;
    node->parent = pivot;
}

void ComboSkillMap::rebalance_after_insert(ComboSkillRbNodeBase* node) {
    while (node != root() &&
           node->parent->color == ComboSkillRbNodeBase::kRed) {
        ComboSkillRbNodeBase* parent = node->parent;
        ComboSkillRbNodeBase* grandparent = parent->parent;
        if (parent == grandparent->left) {
            ComboSkillRbNodeBase* uncle = grandparent->right;
            if (uncle && uncle->color == ComboSkillRbNodeBase::kRed) {
                parent->color = ComboSkillRbNodeBase::kBlack;
                uncle->color = ComboSkillRbNodeBase::kBlack;
                grandparent->color = ComboSkillRbNodeBase::kRed;
                node = grandparent;
            } else {
                if (node == parent->right) {
                    node = parent;
                    rotate_left(node);
                    parent = node->parent;
                    grandparent = parent->parent;
                }
                parent->color = ComboSkillRbNodeBase::kBlack;
                grandparent->color = ComboSkillRbNodeBase::kRed;
                rotate_right(grandparent);
            }
        } else {
            ComboSkillRbNodeBase* uncle = grandparent->left;
            if (uncle && uncle->color == ComboSkillRbNodeBase::kRed) {
                parent->color = ComboSkillRbNodeBase::kBlack;
                uncle->color = ComboSkillRbNodeBase::kBlack;
                grandparent->color = ComboSkillRbNodeBase::kRed;
                node = grandparent;
            } else {
                if (node == parent->left) {
                    node = parent;
                    rotate_right(node);
                    parent = node->parent;
                    grandparent = parent->parent;
                }
                parent->color = ComboSkillRbNodeBase::kBlack;
                grandparent->color = ComboSkillRbNodeBase::kRed;
                rotate_left(grandparent);
            }
        }
    }
    root()->color = ComboSkillRbNodeBase::kBlack;
    root()->parent = &state_.header;
}

ComboSkillMap::iterator ComboSkillMap::begin() {
    return iterator(state_.header.left);
}

ComboSkillMap::const_iterator ComboSkillMap::begin() const {
    return const_iterator(state_.header.left);
}

ComboSkillMap::iterator ComboSkillMap::end() {
    return iterator(&state_.header);
}

ComboSkillMap::const_iterator ComboSkillMap::end() const {
    return const_iterator(&state_.header);
}

ComboSkillMap::iterator ComboSkillMap::find(short key) {
    ComboSkillRbNodeBase* node = root();
    while (node) {
        const short node_key = as_node(node)->value.first;
        if (state_.key_compare(key, node_key)) {
            node = node->left;
        } else if (state_.key_compare(node_key, key)) {
            node = node->right;
        } else {
            return iterator(node);
        }
    }
    return end();
}

ComboSkillMap::const_iterator ComboSkillMap::find(short key) const {
    const ComboSkillRbNodeBase* node = root();
    while (node) {
        const short node_key = as_node(node)->value.first;
        if (state_.key_compare(key, node_key)) {
            node = node->left;
        } else if (state_.key_compare(node_key, key)) {
            node = node->right;
        } else {
            return const_iterator(node);
        }
    }
    return end();
}

std::pair<ComboSkillMap::iterator, bool> ComboSkillMap::insert(
    const value_type& value) {
    ComboSkillRbNodeBase* parent = &state_.header;
    ComboSkillRbNodeBase* node = root();
    bool insert_left = true;
    while (node) {
        parent = node;
        const short node_key = as_node(node)->value.first;
        if (state_.key_compare(value.first, node_key)) {
            insert_left = true;
            node = node->left;
        } else if (state_.key_compare(node_key, value.first)) {
            insert_left = false;
            node = node->right;
        } else {
            return std::make_pair(iterator(node), false);
        }
    }

    ComboSkillRbNode* inserted = new ComboSkillRbNode(value);
    inserted->parent = parent;
    if (parent == &state_.header) {
        root() = inserted;
        state_.header.left = inserted;
        state_.header.right = inserted;
    } else if (insert_left) {
        parent->left = inserted;
        if (parent == state_.header.left) {
            state_.header.left = inserted;
        }
    } else {
        parent->right = inserted;
        if (parent == state_.header.right) {
            state_.header.right = inserted;
        }
    }
    ++state_.node_count;
    rebalance_after_insert(inserted);
    return std::make_pair(iterator(inserted), true);
}

std::pair<ComboSkillMap::iterator, bool> ComboSkillMap::insert(
    const std::pair<short, std::vector<int> >& value) {
    return insert(value_type(value.first, value.second));
}

std::vector<int>& ComboSkillMap::operator[](short key) {
    iterator found = find(key);
    if (found == end()) {
        found = insert(value_type(key, std::vector<int>())).first;
    }
    return found->second;
}

void ComboSkillMap::destroy_subtree(ComboSkillRbNodeBase* node) {
    if (!node) {
        return;
    }
    destroy_subtree(node->left);
    destroy_subtree(node->right);
    delete as_node(node);
}

void ComboSkillMap::clear() {
    destroy_subtree(root());
    root() = 0;
    state_.header.left = &state_.header;
    state_.header.right = &state_.header;
    state_.node_count = 0;
}

bool ComboSkillMap::empty() const {
    return state_.node_count == 0;
}

ComboSkillMap::size_type ComboSkillMap::size() const {
    return state_.node_count;
}

// IDA_EA: 0x848FBBE
// IDA_NAME: _ZN15SIG_COMBO_SKILLC2Ev
// DEMANGLED: SIG_COMBO_SKILL::SIG_COMBO_SKILL(void)
SIG_COMBO_SKILL::SIG_COMBO_SKILL() {
    reset();
}

// IDA_EA: 0x8694D04
// IDA_NAME: _ZN15SIG_COMBO_SKILL5resetEv
// DEMANGLED: SIG_COMBO_SKILL::reset(void)
void* SIG_COMBO_SKILL::reset() {
    charac_no = 0;
    first_tree_count = 0;
    second_tree_count = 0;
    charac_job = oldsrv::config::kCharacterJobInvalid;
    memset(first_tree_combos, 0, sizeof(first_tree_combos));
    return memset(second_tree_combos, 0, sizeof(second_tree_combos));
}

// IDA_EA: 0x80E9644
// IDA_NAME: _ZN10ComboSkill9resetTreeE20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::resetTree(ENUM_SKILL_TREE_KIND)
int ComboSkill::resetTree(::ENUM_SKILL_TREE_KIND kind) {
    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::resetTree(ENUM_SKILL_TREE_KIND)",
            0xD, "ComboSkill::resetTree kind(%d) error", kind);
        return 0;
    }

    ComboSkillMap& tree = *getComboSkillMap(kind);
    for (ComboSkillMap::iterator it = tree.begin(); it != tree.end(); ++it) {
        std::vector<int>& commands = it->second;
        const unsigned int count =
            commands.size() < 6u ? commands.size() : 6u;
        for (unsigned int index = 0; index < count; ++index) {
            commands[index] = 0;
        }
    }
    return 0;
}

// IDA_EA: 0x80E975A
// IDA_NAME: _ZN10ComboSkill20convertComboSkillMapEhPK15comoboSkillData20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::convertComboSkillMap(unsigned char,comoboSkillData const*,ENUM_SKILL_TREE_KIND)
void ComboSkill::convertComboSkillMap(unsigned char count,
                                      const comoboSkillData* data,
                                      ::ENUM_SKILL_TREE_KIND kind) {
    if (!data) {
        return;
    }
    if (count > 6u) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::convertComboSkillMap(unsigned char, const comoboSkillData*, ENUM_SKILL_TREE_KIND)",
            0x25, "ComboSkill::setComboSkill count value(%d) error",
            count);
        return;
    }

    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::convertComboSkillMap(unsigned char, const comoboSkillData*, ENUM_SKILL_TREE_KIND)",
            0x2B, "ComboSkill::setComboSkill kind(%d) error", kind);
        return;
    }

    ComboSkillMap& tree = *getComboSkillMap(kind);
    for (int index = 0; count > index && index <= 5; ++index) {
        const short combo_index = static_cast<short>(data[index].skill_id);
        ComboSkillMap::iterator found = tree.find(combo_index);
        if (found == tree.end()) {
            continue;
        }

        std::vector<int> commands;
        FillSixCommands(commands, data[index].command);
        found->second = commands;
    }
}

// IDA_EA: 0x80E9986
// IDA_NAME: _ZNK10ComboSkill22convertComboSkillArrayERhP15comoboSkillData20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::convertComboSkillArray(unsigned char &,comoboSkillData *,ENUM_SKILL_TREE_KIND)const
void ComboSkill::convertComboSkillArray(unsigned char& count,
                                        comoboSkillData* data,
                                        ::ENUM_SKILL_TREE_KIND kind) const {
    if (!data) {
        return;
    }

    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::convertComboSkillArray(unsigned char&, comoboSkillData*, ENUM_SKILL_TREE_KIND) const",
            0x4B, "ComboSkill::convertComboSkillArray kind(%d) error",
            kind);
        return;
    }

    const ComboSkillMap& tree = *getComboSkillMap(kind);
    int written = 0;
    for (ComboSkillMap::const_iterator it = tree.begin();
         it != tree.end() && written <= 5; ++it, ++written) {
        data[written].skill_id = static_cast<uint16_t>(it->first);
        const std::vector<int>& commands = it->second;
        const unsigned int command_count =
            commands.size() < 6u ? commands.size() : 6u;
        for (unsigned int index = 0; index < command_count; ++index) {
            data[written].command[index] =
                static_cast<uint16_t>(commands[index]);
        }
    }
    count = static_cast<unsigned char>(written);
}

// IDA_EA: 0x80E9B0E
// IDA_NAME: _ZN10ComboSkillC2Ev
// DEMANGLED: ComboSkill::ComboSkill(void)
ComboSkill::ComboSkill()
    : first_tree_(),
      second_tree_() {
    std::vector<int> commands;
    commands.resize(6u, 0);
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        LogManager::logFormat(
            1, "ComboSkill.cpp", "ComboSkill::ComboSkill()", __LINE__,
            "[COMBO_SKILL][CONTAINER_INIT_FAIL] reason=data_manager_null");
        return;
    }
    const std::vector<std::pair<int, int> >& combo_skill_indexes =
        data_manager->get_initial_character_skills(
            oldsrv::config::kCharacterJobDemonicSwordman);
    if (combo_skill_indexes.size() < 6u) {
        LogManager::logFormat(
            1, "ComboSkill.cpp", "ComboSkill::ComboSkill()", __LINE__,
            "[COMBO_SKILL][CONTAINER_INIT_FAIL] reason=container_count count=%u expected=6",
            static_cast<unsigned int>(combo_skill_indexes.size()));
        return;
    }
    for (int index = 0; index <= 5; ++index) {
        const short combo_index =
            static_cast<short>(combo_skill_indexes[index].first);
        first_tree_.insert(
            ComboSkillMap::value_type(combo_index, commands));
        second_tree_.insert(
            ComboSkillMap::value_type(combo_index, commands));
    }
}

// IDA_EA: 0x80E9D26
// IDA_NAME: _ZN10ComboSkillD2Ev
// DEMANGLED: ComboSkill::~ComboSkill()
ComboSkill::~ComboSkill() {}

// IDA_EA: 0x80E9D6C
// IDA_NAME: _ZN10ComboSkill8loadDataEP15SIG_COMBO_SKILL
// DEMANGLED: ComboSkill::loadData(SIG_COMBO_SKILL *)
int ComboSkill::loadData(SIG_COMBO_SKILL* data) {
    if (!data) {
        return 0;
    }
    if (data->first_tree_count) {
        convertComboSkillMap(data->first_tree_count, data->first_tree_combos,
                             ENUM_SKILL_TREE_0);
    }
    if (data->second_tree_count) {
        convertComboSkillMap(data->second_tree_count, data->second_tree_combos,
                             ENUM_SKILL_TREE_1);
    }
    return 1;
}

// IDA_EA: 0x80E9DF2
// IDA_NAME: _ZNK10ComboSkill8saveDataEP15SIG_COMBO_SKILL
// DEMANGLED: ComboSkill::saveData(SIG_COMBO_SKILL *)const
int ComboSkill::saveData(SIG_COMBO_SKILL* data) const {
    if (!data) {
        return 0;
    }
    convertComboSkillArray(data->first_tree_count, data->first_tree_combos,
                           ENUM_SKILL_TREE_0);
    convertComboSkillArray(data->second_tree_count, data->second_tree_combos,
                           ENUM_SKILL_TREE_1);
    return 1;
}

// IDA_EA: 0x80E9E5A
// IDA_NAME: _ZNK10ComboSkill21getComboSkillSendListER11PacketGuard
// DEMANGLED: ComboSkill::getComboSkillSendList(PacketGuard &)const
int ComboSkill::getComboSkillSendList(PacketGuard& packet) const {
    getComboSkillSendForm(packet, ENUM_SKILL_TREE_0);
    return getComboSkillSendForm(packet, ENUM_SKILL_TREE_1);
}

// IDA_EA: 0x80E9E96
// IDA_NAME: _ZNK10ComboSkill21getComboSkillSendFormER11PacketGuard20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::getComboSkillSendForm(PacketGuard &,ENUM_SKILL_TREE_KIND)const
int ComboSkill::getComboSkillSendForm(PacketGuard& packet,
                                      ::ENUM_SKILL_TREE_KIND kind) const {
    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::getComboSkillSendForm(PacketGuard&, ENUM_SKILL_TREE_KIND) const",
            0x99, "ComboSkill::getComboSkillSendForm kind(%d) error",
            kind);
        return 0;
    }

    const ComboSkillMap* tree = getComboSkillMap(kind);
    packet.put_byte(kind);
    packet.put_byte(static_cast<int>(tree->size()));

    for (ComboSkillMap::const_iterator it = tree->begin();
         it != tree->end(); ++it) {
        packet.put_short(it->first);
        const std::vector<int>& commands = it->second;
        packet.put_byte(static_cast<int>(commands.size()));
        const unsigned int count =
            commands.size() < 6u ? commands.size() : 6u;
        for (unsigned int index = 0; index < count; ++index) {
            packet.put_short(commands[index]);
        }
    }
    return 0;
}

// IDA_EA: 0x80EA030
// IDA_NAME: _ZN10ComboSkill13setComboSkillE20ENUM_SKILL_TREE_KINDRSt3mapIsSt6vectorIiSaIiEESt4lessIsESaISt4pairIKsS4_EEE
// DEMANGLED: ComboSkill::setComboSkill(ENUM_SKILL_TREE_KIND,std::map<short,std::vector<int,std::allocator<int>>,std::less<short>,std::allocator<std::pair<short const,std::vector<int,std::allocator<int>>>>> &)
int ComboSkill::setComboSkill(::ENUM_SKILL_TREE_KIND kind,
                              ComboSkillMap& combo_skill_map) {
    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::setComboSkill(ENUM_SKILL_TREE_KIND, ComboSkillMap&)",
            0xB5, "ComboSkill::setComboSkill kind(%d) error", kind);
        return 0;
    }
    if (combo_skill_map.empty()) {
        return 1;
    }

    ComboSkillMap& tree = *getComboSkillMap(kind);
    resetTree(kind);

    for (ComboSkillMap::iterator it = combo_skill_map.begin();
         it != combo_skill_map.end(); ++it) {
        ComboSkillMap::iterator found = tree.find(it->first);
        if (found == tree.end()) {
            continue;
        }
        CopyLimitedCommands(found->second, it->second);
    }
    return 0;
}

// IDA_EA: 0x80EA206
// IDA_NAME: _ZN10ComboSkill26eraseDuplicationComboSKillE20ENUM_SKILL_TREE_KINDi
// DEMANGLED: ComboSkill::eraseDuplicationComboSKill(ENUM_SKILL_TREE_KIND,int)
void ComboSkill::eraseDuplicationComboSKill(::ENUM_SKILL_TREE_KIND kind,
                                            int skill_slot_index) {
    const int tree_index = ComboTreeIndex(kind);
    if (tree_index < 0) {
        LogManager::logFormat(
            1, "ComboSkill.cpp",
            "void ComboSkill::eraseDuplicationComboSKill(ENUM_SKILL_TREE_KIND, int)",
            0xD4, "ComboSkill::eraseDuplicationComboSKill kind(%d) error",
            kind);
        return;
    }

    ComboSkillMap& tree = *getComboSkillMap(kind);
    for (ComboSkillMap::iterator it = tree.begin(); it != tree.end(); ++it) {
        std::vector<int>& commands = it->second;
        for (std::vector<int>::iterator cmd = commands.begin();
             cmd != commands.end();) {
            if (*cmd == skill_slot_index) {
                cmd = commands.erase(cmd);
                commands.push_back(0);
            } else {
                ++cmd;
            }
        }
    }
}

// IDA_EA: 0x80EA3F8
// IDA_NAME: _ZNK10ComboSkill16getComboSkillMapE20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::getComboSkillMap(ENUM_SKILL_TREE_KIND)const
const ComboSkillMap* ComboSkill::getComboSkillMap(
    ::ENUM_SKILL_TREE_KIND kind) const {
    if (kind == ENUM_SKILL_TREE_1) {
        return &second_tree_;
    }
    return &first_tree_;
}

// IDA_EA: 0x80EA41C
// IDA_NAME: _ZN10ComboSkill16getComboSkillMapE20ENUM_SKILL_TREE_KIND
// DEMANGLED: ComboSkill::getComboSkillMap(ENUM_SKILL_TREE_KIND)
ComboSkillMap* ComboSkill::getComboSkillMap(::ENUM_SKILL_TREE_KIND kind) {
    if (kind == ENUM_SKILL_TREE_1) {
        return &second_tree_;
    }
    return &first_tree_;
}

// IDA_EA: 0x86996A0
// IDA_NAME: _ZN10ComboSkillC2ERKS_
// DEMANGLED: ComboSkill::ComboSkill(ComboSkill const&)
ComboSkill::ComboSkill(const ComboSkill& other)
    : first_tree_(other.first_tree_),
      second_tree_(other.second_tree_) {}

// IDA_EA: 0x869D8E8
// IDA_NAME: _ZN10ComboSkillaSERKS_
// DEMANGLED: ComboSkill::operator=(ComboSkill const&)
ComboSkill& ComboSkill::operator=(const ComboSkill& other) {
    if (this != &other) {
        first_tree_ = other.first_tree_;
        second_tree_ = other.second_tree_;
    }
    return *this;
}
