hunter_config(bcos-crypto
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/25c8edb7d5cbadb514bbce9733573c8ffdb3600d.tar.gz"
	SHA1 4a1649e7095f5db58a5ae0671b2278bcccc25f1d
)

hunter_config(bcos-boostssl
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-boostssl/archive/cb2ca607a30012220f5d460471565285833d5da3.tar.gz"
	SHA1 d1a89d9f96e659caa6557dc91178f1fe810e442a
)

hunter_config(bcos-framework VERSION 3.0.0-33486b94
	URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/d4697aa6b984a18e2cab7f91c098480bf5e61ebd.tar.gz
	SHA1 ab2fa84f449aa90b108731734bfd28dc5e78da22
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)
