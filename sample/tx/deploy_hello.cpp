/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file deploy_hello.cpp
 * @author: octopus
 * @date 2022-01-16
 */
#include <bcos-cpp-sdk/SdkFactory.h>
#include <bcos-cpp-sdk/utilities/crypto/CryptoSuite.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPairBuilder.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilder.h>
#include <bcos-utilities/Common.h>
#include <boost/algorithm/string/compare.hpp>
#include <cstdlib>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::boostssl;
using namespace bcos;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// HelloWorld Source Code:
/**
pragma solidity ^0.4.25;

contract HelloWorld{
    string public name;
    constructor() public{
       name = "Hello, World!";
    }

    function set(string n) public{
        name = n;
    }
}
*/
const static std::string hwBIN =
    "608060405234801561001057600080fd5b506040805190810160405280600d81526020017f48656c6c6f2c20576f72"
    "6c6421000000000000000000000000000000000000008152506000908051906020019061005c929190610062565b50"
    "610107565b828054600181600116156101000203166002900490600052602060002090601f01602090048101928260"
    "1f106100a357805160ff19168380011785556100d1565b828001600101855582156100d1579182015b828111156100"
    "d05782518255916020019190600101906100b5565b5b5090506100de91906100e2565b5090565b61010491905b8082"
    "11156101005760008160009055506001016100e8565b5090565b90565b6102d7806101166000396000f30060806040"
    "526004361061004c576000357c0100000000000000000000000000000000000000000000000000000000900463ffff"
    "ffff1680634ed3885e146100515780636d4ce63c146100ba575b600080fd5b34801561005d57600080fd5b506100b8"
    "600480360381019080803590602001908201803590602001908080601f016020809104026020016040519081016040"
    "528093929190818152602001838380828437820191505050505050919291929050505061014a565b005b3480156100"
    "c657600080fd5b506100cf610164565b60405180806020018281038252838181518152602001915080519060200190"
    "80838360005b8381101561010f5780820151818401526020810190506100f4565b50505050905090810190601f1680"
    "1561013c5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b8060"
    "009080519060200190610160929190610206565b5050565b6060600080546001816001161561010002031660029004"
    "80601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203"
    "166002900480156101fc5780601f106101d1576101008083540402835291602001916101fc565b8201919060005260"
    "20600020905b8154815290600101906020018083116101df57829003601f168201915b5050505050905090565b8280"
    "54600181600116156101000203166002900490600052602060002090601f016020900481019282601f106102475780"
    "5160ff1916838001178555610275565b82800160010185558215610275579182015b82811115610274578251825591"
    "602001919060010190610259565b5b5090506102829190610286565b5090565b6102a891905b808211156102a45760"
    "0081600090555060010161028c565b5090565b905600a165627a7a7230582035f32cd11f4fdc4d12abab1850ea5b40"
    "1f853d9ef91e913e4e82d34efa18a2210029";

const static std::string hwSmBIN =
    "608060405234801561001057600080fd5b506040805190810160405280600d81526020017f48656c6c6f2c20576f72"
    "6c6421000000000000000000000000000000000000008152506000908051906020019061005c929190610062565b50"
    "610107565b828054600181600116156101000203166002900490600052602060002090601f01602090048101928260"
    "1f106100a357805160ff19168380011785556100d1565b828001600101855582156100d1579182015b828111156100"
    "d05782518255916020019190600101906100b5565b5b5090506100de91906100e2565b5090565b61010491905b8082"
    "11156101005760008160009055506001016100e8565b5090565b90565b6102d7806101166000396000f30060806040"
    "526004361061004c576000357c0100000000000000000000000000000000000000000000000000000000900463ffff"
    "ffff168063299f7f9d146100515780633590b49f146100e1575b600080fd5b34801561005d57600080fd5b50610066"
    "61014a565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561"
    "00a657808201518184015260208101905061008b565b50505050905090810190601f1680156100d357808203805160"
    "01836020036101000a031916815260200191505b509250505060405180910390f35b3480156100ed57600080fd5b50"
    "610148600480360381019080803590602001908201803590602001908080601f016020809104026020016040519081"
    "01604052809392919081815260200183838082843782019150505050505091929192905050506101ec565b005b6060"
    "60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190"
    "818152602001828054600181600116156101000203166002900480156101e25780601f106101b75761010080835404"
    "02835291602001916101e2565b820191906000526020600020905b8154815290600101906020018083116101c55782"
    "9003601f168201915b5050505050905090565b8060009080519060200190610202929190610206565b5050565b8280"
    "54600181600116156101000203166002900490600052602060002090601f016020900481019282601f106102475780"
    "5160ff1916838001178555610275565b82800160010185558215610275579182015b82811115610274578251825591"
    "602001919060010190610259565b5b5090506102829190610286565b5090565b6102a891905b808211156102a45760"
    "0081600090555060010161028c565b5090565b905600a165627a7a72305820ddb4a8b4c62e003ac9d6ca2396325dce"
    "a3b9441c5d2af668d4ccb883a9af271b0029";

std::string getBinary(bool _sm)
{
    return _sm ? hwSmBIN : hwBIN;
}

void usage()
{
    std::cerr << "Desc: deploy HelloWorld contract\n";
    std::cerr << "Usage: deploy_hello <config> <groupID> <chainID> <SM>\n"
              << "Example:\n"
              << "    ./deploy_hello ./config_sample.ini group0 chain0 false\n"
                 "\n";
    std::exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        usage();
    }

    std::string config = argv[1];
    std::string group = argv[2];
    std::string chainID = argv[3];
    bool sm = boost::iequals(argv[4], "true");

    std::cout << LOG_DESC(" [DeployHello] params ===>>>> ") << LOG_KV("\n\t # config", config)
              << LOG_KV("\n\t # groupID", group) << LOG_KV("\n\t # chainID", chainID)
              << LOG_KV("\n\t # SM", sm) << std::endl;

    auto factory = std::make_shared<SdkFactory>();
    // construct cpp-sdk object
    auto sdk = factory->buildSdk(config);
    // start sdk
    sdk->start();

    std::cout << LOG_DESC(" [DeployHello] start sdk ... ") << std::endl;

    // get group info
    bcos::group::GroupInfo::Ptr groupInfo = sdk->service()->getGroupInfo(group);
    if (!groupInfo)
    {
        std::cout << LOG_DESC(" [DeployHello] group not exist") << LOG_KV("group", group)
                  << std::endl;
        exit(-1);
    }

    auto keyPairBuilder = std::make_shared<bcos::cppsdk::utilities::KeyPairBuilder>();
    auto keyPair =
        keyPairBuilder->genKeyPair(sm ? CryptoSuiteType::SM_TYPE : CryptoSuiteType::ECDSA_TYPE);
    auto cryptoSuite = std::make_shared<bcos::cppsdk::utilities::CryptoSuite>(*keyPair);

    std::cout << LOG_DESC(" [DeployHello] new account ")
              << LOG_KV("address", cryptoSuite->address().hexPrefixed()) << std::endl;

    auto blockLimit = sdk->jsonRpc()->getBlockLimit(group);

    std::cout << LOG_DESC(" [DeployHello] block limit ") << LOG_KV("blockLimit", blockLimit)
              << std::endl;

    auto hexBin = getBinary(sm);
    auto binBytes = fromHexString(hexBin);

    auto transactionBuilder = std::make_shared<bcos::cppsdk::utilities::TransactionBuilder>();
    auto signedTxData = transactionBuilder->createSignedTransaction(
        "", *binBytes.get(), chainID, group, blockLimit, *keyPair);

    std::cout << LOG_DESC(" [DeployHello] create signed transaction success, send data to node ")
              << std::endl;

    std::promise<bool> p;
    auto f = p.get_future();
    sdk->jsonRpc()->sendTransaction(group, "", signedTxData, false,
        [&p](bcos::Error::Ptr _error, std::shared_ptr<bcos::bytes> _resp) {
            if (_error && _error->errorCode() != 0)
            {
                std::cout << LOG_DESC(" [DeployHello] send transaction response error")
                          << LOG_KV("errorCode", _error->errorCode())
                          << LOG_KV("errorMessage", _error->errorMessage()) << std::endl;
            }
            else
            {
                std::cout << LOG_DESC(" [DeployHello] recv response success ") << std::endl;
                std::string receipt = std::string(_resp->begin(), _resp->end());

                Json::Value root;
                Json::Reader jsonReader;

                try
                {
                    if (!jsonReader.parse(receipt, root))
                    {
                        std::cout << LOG_DESC(" [DeployHello] [ERROR] recv invalid json object")
                                  << LOG_KV("resp", receipt) << std::endl;
                        return;
                    }

                    std::cout << LOG_DESC(" [DeployHello] contract address ==> " +
                                          root["result"]["contractAddress"].asString())
                              << std::endl;
                }
                catch (const std::exception& _e)
                {
                    std::cout << LOG_DESC(" [DeployHello] [ERROR] recv invalid json object")
                              << LOG_KV("resp", receipt) << std::endl;
                }
            }
            p.set_value(true);
        });
    f.get();

    return 0;
}