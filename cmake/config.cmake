hunter_config(bcos-crypto
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/25c8edb7d5cbadb514bbce9733573c8ffdb3600d.tar.gz"
	SHA1 4a1649e7095f5db58a5ae0671b2278bcccc25f1d
)

hunter_config(bcos-boostssl
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-boostssl/archive/9b0092385a8271d566bd6d0c392722e031946dba.tar.gz"
	SHA1 9c2f6f6a1fd06c9c1a74ee4e04507c95bd5122d8
)

hunter_config(bcos-framework VERSION 3.0.0-33486b94
	URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/403509babb28e648390dcc5f77576f35e7ba0b7a.tar.gz
	SHA1 f34447aba04532cee17b519a8ce47f3c0e6d1ec1
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)
