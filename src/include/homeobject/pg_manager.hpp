#pragma once
#include <compare>
#include <set>
#include <string>

#include <sisl/utility/enum.hpp>

#include "common.hpp"

namespace homeobject {

ENUM(PGError, uint16_t, UNKNOWN = 1, INVALID_ARG, TIMEOUT, UNKNOWN_PG, UNKNOWN_PEER, UNSUPPORTED_OP);

struct PGMember {
    explicit PGMember(peer_id_t _id) : id(_id) {}
    PGMember(peer_id_t _id, std::string const& _name) : id(_id), name(_name) {}
    PGMember(peer_id_t _id, std::string const& _name, int32_t _priority) : id(_id), name(_name), priority(_priority) {}
    peer_id_t id;
    std::string name;
    int32_t priority{0}; // <0 (Arbiter), ==0 (Follower), >0 (F|Leader)

    auto operator<=>(PGMember const& rhs) const {
        return boost::uuids::hash_value(id) <=> boost::uuids::hash_value(rhs.id);
    }
    auto operator==(PGMember const& rhs) const { return id == rhs.id; }
};

using MemberSet = std::set< PGMember >;

struct PGInfo {
    explicit PGInfo(pg_id_t _id) : id(_id) {}
    pg_id_t id;
    mutable MemberSet members;
    peer_id_t replica_set_uuid;

    auto operator<=>(PGInfo const& rhs) const { return id <=> rhs.id; }
    auto operator==(PGInfo const& rhs) const { return id == rhs.id; }
};

struct PGStats {
    pg_id_t id;
    peer_id_t replica_set_uuid;
    uint32_t num_members;           // number of members in this PG;
    uint32_t total_shards;          // shards allocated on this PG (including open shards)
    uint32_t open_shards;           // active shards on this PG;
    uint32_t available_open_shards; // total number of shards that could be opened on this PG;
    uint64_t used_bytes;            // total number of bytes used by all shards on this PG;
    uint64_t avail_bytes;           // total number of bytes available on this PG;
};

class PGManager : public Manager< PGError > {
public:
    virtual NullAsyncResult create_pg(PGInfo&& pg_info) = 0;
    virtual NullAsyncResult replace_member(pg_id_t id, peer_id_t const& old_member, PGMember const& new_member) = 0;

    /**
     * Retrieves the statistics for a specific PG (Placement Group) identified by its ID.
     *
     * @param id The ID of the PG.
     * @param stats The reference to the PGStats object where the statistics will be stored.
     * @return True if the statistics were successfully retrieved, false otherwise (e.g. id not found).
     */
    virtual bool get_stats(pg_id_t id, PGStats& stats) = 0;

    /**
     * @brief Retrieves the list of pg_ids.
     *
     * This function retrieves the list of pg_ids and stores them in the provided vector.
     *
     * @param pg_ids The vector to store the pg_ids.
     */
    virtual void get_pg_ids(std::vector< pg_id_t >& pg_ids) = 0;
};

} // namespace homeobject
