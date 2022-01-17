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

hunter_config(bcos-utilities VERSION 3.0.0-rc2-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-utilities/archive/d479456ad45c906ad127c1325ebf6e0567e69088.tar.gz
	SHA1 657b739e74629f99a5c33f659f9183683c4ff5b8
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(bcos-crypto VERSION 3.0.0-rc3-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/e2d2af7195a725eeab31da679f12d20c4838e0a1.tar.gz
	SHA1 e7ed758bfbcbd6db3a9d6dd99ba5aca022eab7c5
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON SM2_OPTIMIZE=ON
)

hunter_config(bcos-boostssl
	VERSION 3.0.0-local
    URL https://github.com/LucasLi1024/bcos-boostssl/archive/15240bb022ac9899590ae21b5a44150e69244315.tar.gz
	SHA1 967ee6058d3c9feef87a91fc307ba390b73b9767
)


