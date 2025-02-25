/*
 *  Copyright (C) 2004-2023 Savoir-faire Linux Inc.
 *
 *  Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#pragma once

#include "ice_options.h"
#include "ice_socket.h"
#include "ip_utils.h"

#include <pjnath.h>
#include <pjlib.h>
#include <pjlib-util.h>

#include <functional>
#include <memory>
#include <msgpack.hpp>
#include <vector>

namespace dht {
namespace log {
class Logger;
}
}

namespace jami {

using Logger = dht::log::Logger;

namespace upnp {
class Controller;
}

class IceTransport;

using IceRecvCb = std::function<ssize_t(unsigned char* buf, size_t len)>;
using IceCandidate = pj_ice_sess_cand;
using onShutdownCb = std::function<void(void)>;

struct ICESDP
{
    std::vector<IceCandidate> rem_candidates;
    std::string rem_ufrag;
    std::string rem_pwd;
};

struct SDP
{
    std::string ufrag;
    std::string pwd;

    std::vector<std::string> candidates;
    MSGPACK_DEFINE(ufrag, pwd, candidates)
};

class IceTransport
{
public:
    using Attribute = struct
    {
        std::string ufrag;
        std::string pwd;
    };

    /**
     * Constructor
     */
    IceTransport(std::string_view name);
    ~IceTransport();

    const std::shared_ptr<Logger>& logger() const;

    void initIceInstance(const IceTransportOptions& options);

    /**
     * Get current state
     */
    bool isInitiator() const;

    /**
     * Start transport negotiation between local candidates and given remote
     * to find the right candidate pair.
     * This function doesn't block, the callback on_negodone_cb will be called
     * with the negotiation result when operation is really done.
     * Return false if negotiation cannot be started else true.
     */
    bool startIce(const Attribute& rem_attrs, std::vector<IceCandidate>&& rem_candidates);
    bool startIce(const SDP& sdp);

    /**
     * Cancel operations
     */
    void cancelOperations();

    /**
     * Returns true if ICE transport has been initialized
     * [mutex protected]
     */
    bool isInitialized() const;

    /**
     * Returns true if ICE negotiation has been started
     * [mutex protected]
     */
    bool isStarted() const;

    /**
     * Returns true if ICE negotiation has completed with success
     * [mutex protected]
     */
    bool isRunning() const;

    /**
     * Returns true if ICE transport is in failure state
     * [mutex protected]
     */
    bool isFailed() const;

    IpAddr getLocalAddress(unsigned comp_id) const;

    IpAddr getRemoteAddress(unsigned comp_id) const;

    IpAddr getDefaultLocalAddress() const { return getLocalAddress(1); }

    /**
     * Return ICE session attributes
     */
    const Attribute getLocalAttributes() const;

    /**
     * Return ICE session attributes
     */
    std::vector<std::string> getLocalCandidates(unsigned comp_id) const;

    /**
     * Return ICE session attributes
     */
    std::vector<std::string> getLocalCandidates(unsigned streamIdx, unsigned compId) const;

    bool parseIceAttributeLine(unsigned streamIdx,
                               const std::string& line,
                               IceCandidate& cand) const;

    bool getCandidateFromSDP(const std::string& line, IceCandidate& cand) const;

    // I/O methods

    void setOnRecv(unsigned comp_id, IceRecvCb cb);
    void setOnShutdown(onShutdownCb&& cb);

    ssize_t recv(unsigned comp_id, unsigned char* buf, size_t len, std::error_code& ec);
    ssize_t recvfrom(unsigned comp_id, char* buf, size_t len, std::error_code& ec);

    ssize_t send(unsigned comp_id, const unsigned char* buf, size_t len);

    bool waitForInitialization(std::chrono::milliseconds timeout);

    int waitForNegotiation(std::chrono::milliseconds timeout);

    ssize_t waitForData(unsigned comp_id, std::chrono::milliseconds timeout, std::error_code& ec);

    unsigned getComponentCount() const;

    // Set session state
    bool setSlaveSession();
    bool setInitiatorSession();

    bool isTCPEnabled();

    ICESDP parseIceCandidates(std::string_view sdp_msg);

    void setDefaultRemoteAddress(unsigned comp_id, const IpAddr& addr);

    std::string link() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class IceTransportFactory
{
public:
    IceTransportFactory();
    ~IceTransportFactory();

    std::shared_ptr<IceTransport> createTransport(std::string_view name);

    std::unique_ptr<IceTransport> createUTransport(std::string_view name);

    /**
     * PJSIP specifics
     */
    pj_ice_strans_cfg getIceCfg() const { return ice_cfg_; }
    pj_pool_factory* getPoolFactory() { return &cp_->factory; }
    std::shared_ptr<pj_caching_pool> getPoolCaching() { return cp_; }

private:
    std::shared_ptr<pj_caching_pool> cp_;
    pj_ice_strans_cfg ice_cfg_;
};

}; // namespace jami
