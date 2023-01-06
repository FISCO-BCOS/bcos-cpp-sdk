hunter_config(
    Boost VERSION "1.79.0"
    URL
    "https://osp-1257653870.cos.ap-guangzhou.myqcloud.com/FISCO-BCOS/FISCO-BCOS/deps/boost_1_79_0.tar.bz2
    https://downloads.sourceforge.net/project/boost/boost/1.79.0/source/boost_1_79_0.tar.bz2
    https://nchc.dl.sourceforge.net/project/boost/boost/1.79.0/boost_1_79_0.tar.bz2"
    SHA1
    31209dcff292bd6a64e5e08ceb3ce44a33615dc0
    CMAKE_ARGS
    CONFIG_MACRO=BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
)

hunter_config(
    Microsoft.GSL VERSION "2.0.0-p0-local"
    URL
    "https://osp-1257653870.cos.ap-guangzhou.myqcloud.com/FISCO-BCOS/deps/Microsoft.GSL/v2.0.0-p0.tar.gz
    https://github.com/hunter-packages/Microsoft.GSL/archive/v2.0.0-p0.tar.gz"
    SHA1
    a94c9c1e41edf787a1c080b7cab8f2f4217dbc4b
)

hunter_config(OpenSSL VERSION tassl_1.1.1b_v1.4-local
    URL https://${URL_BASE}/FISCO-BCOS/TASSL-1.1.1b/archive/f9d60fa510e5fbe24413b4abdf1ea3a48f9ee6aa.tar.gz
    SHA1 e56121278bf07587d58d154b4615f96575957d6f
)

hunter_config(bcos-utilities VERSION 1.0.0-rc3-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-utilities/archive/71e57c0172cdda81f4fed6871ab56d9d5dc75fb9.tar.gz
    SHA1 2de5293ae44c7234a7ccd51ba16c8ea223f0ecb8
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(wedpr-crypto VERSION 1.0.0
    URL https://${URL_BASE}/WeBankBlockchain/WeDPR-Lab-Crypto/archive/caeea48d7fa6c879fef52063cf107873744629b2.tar.gz
    SHA1 03e76f0759a0da0f55cad5d49af510657bb6f343
)

hunter_config(bcos-crypto VERSION 1.0.0-rc3-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/221a534f4add4ed4a1aec1889b7d15231d659cc3.tar.gz
    SHA1 ee1337fe3dab573240e6399c426d7929de0492db
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(bcos-boostssl
    VERSION 1.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-boostssl/archive/e8e912b15dc7ea769427a761362f285e9a6611e7.tar.gz
    SHA1 91638e350b922d10a8d573b5b9a214ac447dfbb1
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON ARCH_NATIVE=${ARCH_NATIVE} URL_BASE=${URL_BASE}
)
