#include "dfbridge/repository.hpp"

#ifdef DFBRIDGE_WITH_MYSQL

#include "dfbridge/channel_info.hpp"

#include <mysql.h>

#include <array>
#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

namespace dfbridge {
namespace {

struct ResultDeleter {
    void operator()(MYSQL_RES* result) const noexcept {
        if (result) mysql_free_result(result);
    }
};

using Result = std::unique_ptr<MYSQL_RES, ResultDeleter>;

std::string Cell(MYSQL_ROW row, const unsigned index) {
    return row[index] ? row[index] : "";
}

std::int32_t CellInt(MYSQL_ROW row, const unsigned index) {
    return static_cast<std::int32_t>(std::stol(Cell(row, index)));
}

double CellDouble(MYSQL_ROW row, const unsigned index) {
    return std::stod(Cell(row, index));
}

class MysqlDChannelRepository final : public IDChannelRepository {
public:
    explicit MysqlDChannelRepository(MysqlConfig config) : config_(std::move(config)) {
        Connect();
    }

    ~MysqlDChannelRepository() override {
        if (connection_) mysql_close(connection_);
    }

    void WorkerEnter() override {
        if (mysql_thread_init() != 0) {
            throw std::runtime_error("mysql_thread_init failed");
        }
    }

    void WorkerLeave() noexcept override {
        mysql_thread_end();
    }

    std::string ReadScriptVersion() override {
        auto result = Query("SELECT script_version FROM ch_script_version LIMIT 1");
        MYSQL_ROW row = mysql_fetch_row(result.get());
        if (!row) throw std::runtime_error("ch_script_version is empty");
        return Cell(row, 0);
    }

    ChannelSnapshot LoadSnapshot() override {
        ChannelSnapshot snapshot;
        snapshot.script_version = ReadScriptVersion();

        {
            auto result = Query("SELECT kind_name,dungeon_name FROM ch_dungeon_data");
            while (MYSQL_ROW row = mysql_fetch_row(result.get())) {
                snapshot.dungeons.push_back({Cell(row, 0), Cell(row, 1), {}});
            }
        }
        {
            std::map<std::string, std::vector<std::int32_t>> ids;
            auto result = Query(
                "SELECT kind_name,dungeon_id FROM ch_dungeon_list "
                "ORDER BY kind_name,dungeon_id");
            while (MYSQL_ROW row = mysql_fetch_row(result.get())) {
                ids[NormalizeKindName(Cell(row, 0))].push_back(CellInt(row, 1));
            }
            for (auto& dungeon : snapshot.dungeons) {
                const auto found = ids.find(NormalizeKindName(dungeon.kind_name));
                if (found != ids.end()) dungeon.dungeon_ids = found->second;
            }
        }
        {
            // Named columns intentionally replace the fragile original SELECT *.
            // The schema remains the same 17 database columns / 15 emitted
            // business values, including channel_kind 19, 20, and 23.
            auto result = Query(
                "SELECT server_id,is_use,channel_number,channel_name,channel_kind,kind_name,"
                "exp_rate,exp_1,exp_2,exp_3,exp_4,exp_5,exp_6,exp_7,exp_8,exp_9,exp_10 "
                "FROM ch_server_data WHERE is_use=1 ORDER BY server_id,channel_number");
            while (MYSQL_ROW row = mysql_fetch_row(result.get())) {
                ChannelRow channel;
                channel.server_group = CellInt(row, 0);
                channel.is_use = CellInt(row, 1) != 0;
                channel.channel_number = CellInt(row, 2);
                channel.channel_name = Cell(row, 3);
                channel.channel_kind = CellInt(row, 4);
                channel.kind_name = Cell(row, 5);
                channel.exp_rate = CellDouble(row, 6);
                for (unsigned i = 0; i < channel.exp_bonus.size(); ++i) {
                    channel.exp_bonus[i] = CellDouble(row, 7 + i);
                }
                snapshot.channels.push_back(std::move(channel));
            }
        }
        {
            auto result = Query(
                "SELECT group_name,group_gc_no FROM ch_gc_info ORDER BY group_gc_no ASC");
            while (MYSQL_ROW row = mysql_fetch_row(result.get())) {
                snapshot.gc_groups.push_back({Cell(row, 0), CellInt(row, 1)});
            }
        }
        ValidateSnapshot(snapshot);
        return snapshot;
    }

private:
    void Connect() {
        if (connection_) {
            mysql_close(connection_);
            connection_ = nullptr;
        }
        connection_ = mysql_init(nullptr);
        if (!connection_) throw std::runtime_error("mysql_init failed");
        my_bool reconnect = 1;
        mysql_options(connection_, MYSQL_OPT_RECONNECT, &reconnect);
        if (!mysql_real_connect(connection_, config_.host.c_str(), config_.user.c_str(),
                                config_.password.c_str(), config_.database.c_str(),
                                config_.port, nullptr, 0)) {
            const std::string message = mysql_error(connection_);
            mysql_close(connection_);
            connection_ = nullptr;
            throw std::runtime_error("mysql_real_connect failed: " + message);
        }
    }

    Result Query(const char* sql) {
        if (!connection_ || mysql_ping(connection_) != 0) {
            Connect();
        }
        int query_result = mysql_query(connection_, sql);
        if (query_result != 0) {
            // Retry once with a fresh handle for a dropped/expired connection.
            Connect();
            query_result = mysql_query(connection_, sql);
        }
        if (query_result != 0) {
            throw std::runtime_error(std::string("d_channel query failed: ") +
                                     mysql_error(connection_));
        }
        Result result(mysql_store_result(connection_));
        if (!result) {
            throw std::runtime_error(std::string("mysql_store_result failed: ") +
                                     mysql_error(connection_));
        }
        return result;
    }

    MysqlConfig config_;
    MYSQL* connection_{};
};

}  // namespace

std::unique_ptr<IDChannelRepository> MakeMysqlRepository(MysqlConfig config) {
    return std::unique_ptr<IDChannelRepository>(new MysqlDChannelRepository(std::move(config)));
}

}  // namespace dfbridge

#endif
