hunter_config(bcos-crypto
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/25c8edb7d5cbadb514bbce9733573c8ffdb3600d.tar.gz"
	SHA1 4a1649e7095f5db58a5ae0671b2278bcccc25f1d
)

hunter_config(bcos-boostssl
	VERSION 3.0.0-local
	URL "https://${URL_BASE}/FISCO-BCOS/bcos-boostssl/archive/176dbb74c41a0efaf38bd8725908929b708bdf14.tar.gz"
	SHA1 ab851a11610fab6df520730a3bcbf8bd08a87065
)

hunter_config(bcos-framework VERSION 3.0.0-33486b94
	URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/65333506afa59cc0470444f46d025d4a3dc92c89.tar.gz
	SHA1 0f15fd99003ccb8b25bd1c5203d43017877c4000
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)
