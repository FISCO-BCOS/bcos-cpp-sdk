hunter_config(
    Boost VERSION "1.76.0-local"
    URL
    "https://osp-1257653870.cos.ap-guangzhou.myqcloud.com/FISCO-BCOS/FISCO-BCOS/deps/boost_1_76_0.tar.bz2
    https://downloads.sourceforge.net/project/boost/boost/1.76.0/source/boost_1_76_0.tar.bz2
    https://nchc.dl.sourceforge.net/project/boost/boost/1.76.0/boost_1_76_0.tar.bz2"
    SHA1
    8064156508312dde1d834fec3dca9b11006555b6
    CMAKE_ARGS
    CONFIG_MACRO=BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
)

hunter_config(OpenSSL VERSION tassl_1.1.1b_v1.4-local
    URL https://${URL_BASE}/FISCO-BCOS/TASSL-1.1.1b/archive/6a0fddd0eb33433c190c796e5b6d80db4db52810.tar.gz
    SHA1 14f7590e09b54bb71926ad5853343f20893ee536
)

hunter_config(bcos-utilities VERSION 1.0.0-rc1-f12788a1 CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON)

hunter_config(bcos-crypto VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/eb042091bb6aef907aecfe84dfe365da006b2952.tar.gz
    SHA1 dbfe8d9f79311fd62c8f69e0f870b1150077f4ed
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON SM2_OPTIMIZE=ON
)

hunter_config(bcos-boostssl
	VERSION 1.0.0-rc3-local
	URL https://${URL_BASE}/FISCO-BCOS/bcos-boostssl/archive/3a1a5889a160e9a4fbaa41acd546c85acdf2426e.tar.gz
	SHA1 46a4e1ba4a089136a1f9e88a95934c2c6eeaf295
)