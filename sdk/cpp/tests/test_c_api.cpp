//////////////////////////////////////////////////////////////////
// @file golang.cpp
//
// YANG Development Kit
// Copyright 2017 Cisco Systems. All rights reserved
//
////////////////////////////////////////////////////////////////
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
//////////////////////////////////////////////////////////////////

#include <ydk/ydk.h>
#include "catch.hpp"

using namespace std;

const char* test_string="<runner xmlns=\"http://cisco.com/ns/yang/ydktest-sanity\"><ytypes><built-in-t><number8>2</number8></built-in-t></ytypes></runner>";

TEST_CASE( "codec_encode"  )
{
    YDKStatePtr state = YDKStateCreate();
    Codec c = CodecInit();
    Repository repo = RepositoryInitWithPath(state, "/usr/local/share/ydktest@0.1.0");
    ServiceProvider provider = NetconfServiceProviderInitWithRepo(state, repo, "localhost", "admin", "admin", 12022, "ssh");

    RootSchemaNode root_schema = ServiceProviderGetRootSchema(state, provider);

    DataNode runner = RootSchemaNodeCreate(state, root_schema, "ydktest-sanity:runner");

    DataNodeCreate(state, runner, "ytypes/built-in-t/number8", "2");

    string s { CodecEncode(state, c, runner, XML, 0)};
    REQUIRE(s == test_string);

    NetconfServiceProviderFree(provider);
    RepositoryFree(repo);
    CodecFree(c);
    YDKStateFree(state);
}

TEST_CASE( "codec_decode"  )
{
    YDKStatePtr state = YDKStateCreate();
    Codec c = CodecInit();
    Repository repo = RepositoryInitWithPath(state, "/usr/local/share/ydktest@0.1.0");
    ServiceProvider provider = NetconfServiceProviderInitWithRepo(state, repo, "localhost", "admin", "admin", 12022, "ssh");

    RootSchemaNode root_schema = ServiceProviderGetRootSchema(state, provider);

    DataNode runner = CodecDecode(state, c, root_schema, test_string, XML);

    REQUIRE(runner!=NULL);

    string s { CodecEncode(state, c, runner, XML, 0)};
    REQUIRE(s == test_string);

    NetconfServiceProviderFree(provider);
    RepositoryFree(repo);
    CodecFree(c);
    YDKStateFree(state);
}

TEST_CASE( "provider_withpath"  )
{
    YDKStatePtr state = YDKStateCreate();
    Repository repo = RepositoryInitWithPath(state, "/usr/local/share/ydktest@0.1.0");
    ServiceProvider provider = NetconfServiceProviderInitWithRepo(state, repo, "localhost", "admin", "admin", 12022, "ssh");

    REQUIRE(repo!=NULL);
    REQUIRE(provider!=NULL);

    NetconfServiceProviderFree(provider);
    RepositoryFree(repo);
    YDKStateFree(state);
}

TEST_CASE( "provider"  )
{
    YDKStatePtr state = YDKStateCreate();
    Repository repo = RepositoryInit();
    ServiceProvider provider = NetconfServiceProviderInitWithRepo(state, repo, "localhost", "admin", "admin", 12022, "ssh");

    REQUIRE(repo!=NULL);
    REQUIRE(provider!=NULL);

    NetconfServiceProviderFree(provider);
    RepositoryFree(repo);
    YDKStateFree(state);
}

TEST_CASE( "rpc" )
{
    YDKStatePtr state = YDKStateCreate();
    Codec c = CodecInit();

    Repository repo = RepositoryInitWithPath(state, "/usr/local/share/ydktest@0.1.0");

    ServiceProvider provider = NetconfServiceProviderInitWithRepo(state, repo, "localhost", "admin", "admin", 12022, "ssh");
    RootSchemaNode root_schema = ServiceProviderGetRootSchema(state, provider);

    DataNode runner = RootSchemaNodeCreate(state, root_schema, "ydktest-sanity:runner");

    DataNodeCreate(state, runner, "ytypes/built-in-t/number8", "2");
    const char* create_xml = CodecEncode(state, c, runner, XML, 0);

    Rpc create_rpc = RootSchemaNodeRpc(state, root_schema, "ydk:create");
    DataNode input = RpcInput(state, create_rpc);
    DataNodeCreate(state, input, "entity", create_xml);
    RpcExecute(state, create_rpc, provider);

    Rpc read_rpc = RootSchemaNodeRpc(state, root_schema, "ydk:read");
    input = RpcInput(state, read_rpc);
    DataNode runner_filter = RootSchemaNodeCreate(state, root_schema, "ydktest-sanity:runner");
    const char* read_xml = CodecEncode(state, c, runner_filter, XML, 0);

    DataNodeCreate(state, input, "filter", read_xml);
    DataNode read_data = RpcExecute(state, read_rpc, provider);

    delete create_xml;
    delete read_xml;
    NetconfServiceProviderFree(provider);
    RepositoryFree(repo);
    CodecFree(c);
    YDKStateFree(state);
}
