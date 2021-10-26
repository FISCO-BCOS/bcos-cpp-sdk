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
	URL https://${URL_BASE}/ywy2090/bcos-framework/archive/be46e2d1c39c3f5bef3eccbab682c04233c58888.tar.gz
	SHA1 c0ec3bcbaa636f4c5e5af7d158045f7b2ade1fce
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)
