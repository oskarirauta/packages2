#ifndef EFIVAR_BUILD_ENVIRONMENT
#define EFIVAR_BUILD_ENVIRONMENT
#endif /* EFIVAR_BUILD_ENVIRONMENT */

#include "fix_coverity.h"
#include <efivar/efivar.h>
#include <endian.h>

#if BYTE_ORDER == BIG_ENDIAN
#define cpu_to_be32(n) (n)
#define cpu_to_be16(n) (n)
#define cpu_to_le32(n) (__builtin_bswap32(n))
#define cpu_to_le16(n) (__builtin_bswap16(n))
#else
#define cpu_to_le32(n) (n)
#define cpu_to_le16(n) (n)
#define cpu_to_be32(n) (__builtin_bswap32(n))
#define cpu_to_be16(n) (__builtin_bswap16(n))
#endif

const efi_guid_t
	__attribute__((__visibility__ ("default")))
	efi_guid_empty = {cpu_to_le32(0x00000000),cpu_to_le16(0x0000),cpu_to_le16(0x0000),cpu_to_be16(0x0000),{0x00,0x00,0x00,0x00,0x00,0x00}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_zero = {cpu_to_le32(0x00000000),cpu_to_le16(0x0000),cpu_to_le16(0x0000),cpu_to_be16(0x0000),{0x00,0x00,0x00,0x00,0x00,0x00}};


const efi_guid_t
	__attribute__((__visibility__ ("default")))
	efi_guid_redhat_2 = {cpu_to_le32(0x0223eddb),cpu_to_le16(0x9079),cpu_to_le16(0x4388),cpu_to_be16(0xaf77),{0x2d,0x65,0xb1,0xc3,0x5d,0x3b}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_redhat = {cpu_to_le32(0x0223eddb),cpu_to_le16(0x9079),cpu_to_le16(0x4388),cpu_to_be16(0xaf77),{0x2d,0x65,0xb1,0xc3,0x5d,0x3b}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_sha512 = {cpu_to_le32(0x093e0fae),cpu_to_le16(0xa6c4),cpu_to_le16(0x4f50),cpu_to_be16(0x9f1b),{0xd4,0x1e,0x2b,0x89,0xc1,0x9a}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_fwupdate = {cpu_to_le32(0x0abba7dc),cpu_to_le16(0xe516),cpu_to_le16(0x4167),cpu_to_be16(0xbbf5),{0x4d,0x9d,0x1c,0x73,0x94,0x16}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_sha224 = {cpu_to_le32(0x0b6e5233),cpu_to_le16(0xa65c),cpu_to_le16(0x44c9),cpu_to_be16(0x9407),{0xd9,0xab,0x83,0xbf,0xc8,0xbd}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_boot_menu = {cpu_to_le32(0x126a762d),cpu_to_le16(0x5758),cpu_to_le16(0x4fca),cpu_to_be16(0x8531),{0x20,0x1a,0x7f,0x57,0xf8,0x50}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_supermicro = {cpu_to_le32(0x26dc4851),cpu_to_le16(0x195f),cpu_to_le16(0x4ae1),cpu_to_be16(0x9a19),{0xfb,0xf8,0x83,0xbb,0xb3,0x5e}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_asus = {cpu_to_le32(0x3b053091),cpu_to_le16(0x6c9f),cpu_to_le16(0x04cc),cpu_to_be16(0xb1ac),{0xe2,0xa5,0x1e,0x3b,0xe5,0xf5}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_ux_capsule = {cpu_to_le32(0x3b8c8162),cpu_to_le16(0x188c),cpu_to_le16(0x46a4),cpu_to_be16(0xaec9),{0xbe,0x43,0xf1,0xd6,0x56,0x97}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_x509_sha256 = {cpu_to_le32(0x3bd2a492),cpu_to_le16(0x96c0),cpu_to_le16(0x4079),cpu_to_be16(0xb420),{0xfc,0xf9,0x8e,0xf1,0x03,0xed}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_rsa2048 = {cpu_to_le32(0x3c5766e8),cpu_to_le16(0x269c),cpu_to_le16(0x4e34),cpu_to_be16(0xaa14),{0xed,0x77,0x6e,0x85,0xb3,0xb6}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo = {cpu_to_le32(0x3cc24e96),cpu_to_le16(0x22c7),cpu_to_le16(0x41d8),cpu_to_be16(0x8863),{0x8e,0x39,0xdc,0xdc,0xc2,0xcf}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_diag = {cpu_to_le32(0x3f7e615b),cpu_to_le16(0x0d45),cpu_to_le16(0x4f80),cpu_to_be16(0x88dc),{0x26,0xb2,0x34,0x95,0x85,0x60}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_x509_sha512 = {cpu_to_le32(0x446dbf63),cpu_to_le16(0x2502),cpu_to_le16(0x4cda),cpu_to_be16(0xbcfa),{0x24,0x65,0xd2,0xb0,0xfe,0x9d}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_external_management = {cpu_to_le32(0x452e8ced),cpu_to_le16(0xdfff),cpu_to_le16(0x4b8c),cpu_to_be16(0xae01),{0x51,0x18,0x86,0x2e,0x68,0x2c}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_pkcs7_cert = {cpu_to_le32(0x4aafd29d),cpu_to_le16(0x68df),cpu_to_le16(0x49ee),cpu_to_be16(0x8aa9),{0x34,0x7d,0x37,0x56,0x65,0xa7}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_fives = {cpu_to_le32(0x55555555),cpu_to_le16(0x5555),cpu_to_le16(0x5555),cpu_to_be16(0x5555),{0x55,0x55,0x55,0x55,0x55,0x55}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_shim = {cpu_to_le32(0x605dab50),cpu_to_le16(0xe046),cpu_to_le16(0x4300),cpu_to_be16(0xabb6),{0x3d,0xd8,0x10,0xdd,0x8b,0x23}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_rescue = {cpu_to_le32(0x665d3f60),cpu_to_le16(0xad3e),cpu_to_le16(0x4cad),cpu_to_be16(0x8e26),{0xdb,0x46,0xee,0xe9,0xf1,0xb5}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_rsa2048_sha1 = {cpu_to_le32(0x67f8444f),cpu_to_le16(0x8743),cpu_to_le16(0x48f1),cpu_to_be16(0xa328),{0x1e,0xaa,0xb8,0x73,0x60,0x80}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_canonical = {cpu_to_le32(0x6dc40ae4),cpu_to_le16(0x2ee8),cpu_to_le16(0x9c4c),cpu_to_be16(0xa314),{0x0f,0xc7,0xb2,0x00,0x87,0x10}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_dell = {cpu_to_le32(0x70564dce),cpu_to_le16(0x9afc),cpu_to_le16(0x4ee3),cpu_to_be16(0x85fc),{0x94,0x96,0x49,0xd7,0xe4,0x5c}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_x509_sha384 = {cpu_to_le32(0x7076876e),cpu_to_le16(0x80c2),cpu_to_le16(0x4ee6),cpu_to_be16(0xaad2),{0x28,0xb3,0x49,0xa6,0x86,0x5b}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_setup = {cpu_to_le32(0x721c8b66),cpu_to_le16(0x426c),cpu_to_le16(0x4e86),cpu_to_be16(0x8e99),{0x34,0x57,0xc4,0x6a,0xb0,0xb9}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_microsoft = {cpu_to_le32(0x77fa9abd),cpu_to_le16(0x0359),cpu_to_le16(0x4d32),cpu_to_be16(0xbd60),{0x28,0xf4,0xe7,0x8f,0x78,0x4b}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_2 = {cpu_to_le32(0x7facc7b6),cpu_to_le16(0x127f),cpu_to_le16(0x4e9c),cpu_to_be16(0x9c5d),{0x08,0x0f,0x98,0x99,0x43,0x45}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_auto_created_boot_option = {cpu_to_le32(0x8108ac4e),cpu_to_le16(0x9f11),cpu_to_le16(0x4d59),cpu_to_be16(0x850e),{0xe2,0x1a,0x52,0x2c,0x59,0xb2}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_sha1 = {cpu_to_le32(0x826ca512),cpu_to_le16(0xcf10),cpu_to_le16(0x4ac9),cpu_to_be16(0xb187),{0xbe,0x01,0x49,0x66,0x31,0xbd}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_me_config = {cpu_to_le32(0x82988420),cpu_to_le16(0x7467),cpu_to_le16(0x4490),cpu_to_be16(0x9059),{0xfe,0xb4,0x48,0xdd,0x19,0x63}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_global = {cpu_to_le32(0x8be4df61),cpu_to_le16(0x93ca),cpu_to_le16(0x11d2),cpu_to_be16(0xaa0d),{0x00,0xe0,0x98,0x03,0x2b,0x8c}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_grub = {cpu_to_le32(0x91376aff),cpu_to_le16(0xcba6),cpu_to_le16(0x42be),cpu_to_be16(0x949d),{0x06,0xfd,0xe8,0x11,0x28,0xe8}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_x509_cert = {cpu_to_le32(0xa5c059a1),cpu_to_le16(0x94e4),cpu_to_le16(0x4aa7),cpu_to_be16(0x87b5),{0xab,0x15,0x5c,0x2b,0xf0,0x72}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_rsa2048_sha256_cert = {cpu_to_le32(0xa7717414),cpu_to_le16(0xc616),cpu_to_le16(0x4977),cpu_to_be16(0x9420),{0x84,0x47,0x12,0xa7,0x35,0xbf}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_diag_splash = {cpu_to_le32(0xa7d8d9a6),cpu_to_le16(0x6ab0),cpu_to_le16(0x4aeb),cpu_to_be16(0xad9d),{0x16,0x3e,0x59,0xa7,0xa3,0x80}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_msg = {cpu_to_le32(0xbc7838d2),cpu_to_le16(0x0f82),cpu_to_le16(0x4d60),cpu_to_be16(0x8316),{0xc0,0x68,0xee,0x79,0xd2,0x5b}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_sha256 = {cpu_to_le32(0xc1c41626),cpu_to_le16(0x504c),cpu_to_le16(0x4092),cpu_to_be16(0xaca9),{0x41,0xf9,0x36,0x93,0x43,0x28}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_shell = {cpu_to_le32(0xc57ad6b7),cpu_to_le16(0x0515),cpu_to_le16(0x40a8),cpu_to_be16(0x9d21),{0x55,0x16,0x52,0x85,0x4e,0x37}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_security = {cpu_to_le32(0xd719b2cb),cpu_to_le16(0x3d3a),cpu_to_le16(0x4596),cpu_to_be16(0xa3bc),{0xda,0xd0,0x0e,0x67,0x65,0x6f}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_rsa2048_sha256 = {cpu_to_le32(0xe2b36190),cpu_to_le16(0x879b),cpu_to_le16(0x4a3d),cpu_to_be16(0xad8d),{0xf2,0xe7,0xbb,0xa3,0x27,0x84}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_lenovo_startup_interrupt = {cpu_to_le32(0xf46ee6f4),cpu_to_le16(0x4785),cpu_to_le16(0x43a3),cpu_to_be16(0x923d),{0x7f,0x78,0x6c,0x3c,0x84,0x79}};

const efi_guid_t
__attribute__((__visibility__ ("default")))
	efi_guid_sha384 = {cpu_to_le32(0xff3e5307),cpu_to_le16(0x9fd0),cpu_to_le16(0x48c9),cpu_to_be16(0x85f1),{0x8a,0xd5,0x6c,0x70,0x1e,0x01}};

const uint64_t
	__attribute__((__visibility__ ("default")))
	efi_n_well_known_guids = 41;
const uint64_t
	__attribute__((__visibility__ ("default")))
	efi_n_well_known_names = 41;

struct efivar_guidname {
	efi_guid_t guid;
	char symbol[256];
	char name[256];
	char description[256];
} __attribute__((__aligned__(16)));

const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_guids_[42]= {
		{.guid={.a=cpu_to_le32(0),
		        .b=cpu_to_le16(0),
		        .c=cpu_to_le16(0),
		        .d=cpu_to_be16(0x0000),
		        .e={0,0,0,0,0,0}},
		 .symbol="efi_guid_zero",
		 .name="zero",
		 .description="zeroed sentinel guid",
		},
		{.guid={.a=cpu_to_le32(0x223eddb),
		        .b=cpu_to_le16(0x9079),
		        .c=cpu_to_le16(0x4388),
		        .d=cpu_to_be16(0xaf77),
		        .e={0x2d,0x65,0xb1,0xc3,0x5d,0x3b}},
		 .symbol="efi_guid_redhat",
		 .name="redhat",
		 .description="Red Hat",
		},
		{.guid={.a=cpu_to_le32(0x93e0fae),
		        .b=cpu_to_le16(0xa6c4),
		        .c=cpu_to_le16(0x4f50),
		        .d=cpu_to_be16(0x9f1b),
		        .e={0xd4,0x1e,0x2b,0x89,0xc1,0x9a}},
		 .symbol="efi_guid_sha512",
		 .name="sha512",
		 .description="SHA-512 hash",
		},
		{.guid={.a=cpu_to_le32(0xabba7dc),
		        .b=cpu_to_le16(0xe516),
		        .c=cpu_to_le16(0x4167),
		        .d=cpu_to_be16(0xbbf5),
		        .e={0x4d,0x9d,0x1c,0x73,0x94,0x16}},
		 .symbol="efi_guid_fwupdate",
		 .name="fwupdate",
		 .description="Linux Firmware Update Tool",
		},
		{.guid={.a=cpu_to_le32(0xb6e5233),
		        .b=cpu_to_le16(0xa65c),
		        .c=cpu_to_le16(0x44c9),
		        .d=cpu_to_be16(0x9407),
		        .e={0xd9,0xab,0x83,0xbf,0xc8,0xbd}},
		 .symbol="efi_guid_sha224",
		 .name="sha224",
		 .description="SHA-224 hash",
		},
		{.guid={.a=cpu_to_le32(0x126a762d),
		        .b=cpu_to_le16(0x5758),
		        .c=cpu_to_le16(0x4fca),
		        .d=cpu_to_be16(0x8531),
		        .e={0x20,0x1a,0x7f,0x57,0xf8,0x50}},
		 .symbol="efi_guid_lenovo_boot_menu",
		 .name="lenovo_boot_menu",
		 .description="Lenovo Boot Menu",
		},
		{.guid={.a=cpu_to_le32(0x26dc4851),
		        .b=cpu_to_le16(0x195f),
		        .c=cpu_to_le16(0x4ae1),
		        .d=cpu_to_be16(0x9a19),
		        .e={0xfb,0xf8,0x83,0xbb,0xb3,0x5e}},
		 .symbol="efi_guid_supermicro",
		 .name="supermicro",
		 .description="Super Micro",
		},
		{.guid={.a=cpu_to_le32(0x3b053091),
		        .b=cpu_to_le16(0x6c9f),
		        .c=cpu_to_le16(0x4cc),
		        .d=cpu_to_be16(0xb1ac),
		        .e={0xe2,0xa5,0x1e,0x3b,0xe5,0xf5}},
		 .symbol="efi_guid_asus",
		 .name="asus",
		 .description="Asus",
		},
		{.guid={.a=cpu_to_le32(0x3b8c8162),
		        .b=cpu_to_le16(0x188c),
		        .c=cpu_to_le16(0x46a4),
		        .d=cpu_to_be16(0xaec9),
		        .e={0xbe,0x43,0xf1,0xd6,0x56,0x97}},
		 .symbol="efi_guid_ux_capsule",
		 .name="ux_capsule",
		 .description="Firmware update localized text image",
		},
		{.guid={.a=cpu_to_le32(0x3bd2a492),
		        .b=cpu_to_le16(0x96c0),
		        .c=cpu_to_le16(0x4079),
		        .d=cpu_to_be16(0xb420),
		        .e={0xfc,0xf9,0x8e,0xf1,0x3,0xed}},
		 .symbol="efi_guid_x509_sha256",
		 .name="x509_sha256",
		 .description="SHA-256 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x3c5766e8),
		        .b=cpu_to_le16(0x269c),
		        .c=cpu_to_le16(0x4e34),
		        .d=cpu_to_be16(0xaa14),
		        .e={0xed,0x77,0x6e,0x85,0xb3,0xb6}},
		 .symbol="efi_guid_rsa2048",
		 .name="rsa2048",
		 .description="RSA 2048 pubkey",
		},
		{.guid={.a=cpu_to_le32(0x3cc24e96),
		        .b=cpu_to_le16(0x22c7),
		        .c=cpu_to_le16(0x41d8),
		        .d=cpu_to_be16(0x8863),
		        .e={0x8e,0x39,0xdc,0xdc,0xc2,0xcf}},
		 .symbol="efi_guid_lenovo",
		 .name="lenovo",
		 .description="Lenovo",
		},
		{.guid={.a=cpu_to_le32(0x3f7e615b),
		        .b=cpu_to_le16(0xd45),
		        .c=cpu_to_le16(0x4f80),
		        .d=cpu_to_be16(0x88dc),
		        .e={0x26,0xb2,0x34,0x95,0x85,0x60}},
		 .symbol="efi_guid_lenovo_diag",
		 .name="lenovo_diag",
		 .description="Lenovo Diagnostics",
		},
		{.guid={.a=cpu_to_le32(0x446dbf63),
		        .b=cpu_to_le16(0x2502),
		        .c=cpu_to_le16(0x4cda),
		        .d=cpu_to_be16(0xbcfa),
		        .e={0x24,0x65,0xd2,0xb0,0xfe,0x9d}},
		 .symbol="efi_guid_x509_sha512",
		 .name="x509_sha512",
		 .description="SHA-512 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x452e8ced),
		        .b=cpu_to_le16(0xdfff),
		        .c=cpu_to_le16(0x4b8c),
		        .d=cpu_to_be16(0xae01),
		        .e={0x51,0x18,0x86,0x2e,0x68,0x2c}},
		 .symbol="efi_guid_external_management",
		 .name="external_management",
		 .description="External Management Mechanism",
		},
		{.guid={.a=cpu_to_le32(0x4aafd29d),
		        .b=cpu_to_le16(0x68df),
		        .c=cpu_to_le16(0x49ee),
		        .d=cpu_to_be16(0x8aa9),
		        .e={0x34,0x7d,0x37,0x56,0x65,0xa7}},
		 .symbol="efi_guid_pkcs7_cert",
		 .name="pkcs7_cert",
		 .description="PKCS7 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x55555555),
		        .b=cpu_to_le16(0x5555),
		        .c=cpu_to_le16(0x5555),
		        .d=cpu_to_be16(0x5555),
		        .e={0x55,0x55,0x55,0x55,0x55,0x55}},
		 .symbol="efi_guid_fives",
		 .name="fives",
		 .description="All Fives Test Data",
		},
		{.guid={.a=cpu_to_le32(0x605dab50),
		        .b=cpu_to_le16(0xe046),
		        .c=cpu_to_le16(0x4300),
		        .d=cpu_to_be16(0xabb6),
		        .e={0x3d,0xd8,0x10,0xdd,0x8b,0x23}},
		 .symbol="efi_guid_shim",
		 .name="shim",
		 .description="shim",
		},
		{.guid={.a=cpu_to_le32(0x665d3f60),
		        .b=cpu_to_le16(0xad3e),
		        .c=cpu_to_le16(0x4cad),
		        .d=cpu_to_be16(0x8e26),
		        .e={0xdb,0x46,0xee,0xe9,0xf1,0xb5}},
		 .symbol="efi_guid_lenovo_rescue",
		 .name="lenovo_rescue",
		 .description="Lenovo Rescue and Recovery",
		},
		{.guid={.a=cpu_to_le32(0x67f8444f),
		        .b=cpu_to_le16(0x8743),
		        .c=cpu_to_le16(0x48f1),
		        .d=cpu_to_be16(0xa328),
		        .e={0x1e,0xaa,0xb8,0x73,0x60,0x80}},
		 .symbol="efi_guid_rsa2048_sha1",
		 .name="rsa2048_sha1",
		 .description="RSA-2048 signature of a SHA-1 hash",
		},
		{.guid={.a=cpu_to_le32(0x6dc40ae4),
		        .b=cpu_to_le16(0x2ee8),
		        .c=cpu_to_le16(0x9c4c),
		        .d=cpu_to_be16(0xa314),
		        .e={0xf,0xc7,0xb2,0,0x87,0x10}},
		 .symbol="efi_guid_canonical",
		 .name="canonical",
		 .description="Canonical",
		},
		{.guid={.a=cpu_to_le32(0x70564dce),
		        .b=cpu_to_le16(0x9afc),
		        .c=cpu_to_le16(0x4ee3),
		        .d=cpu_to_be16(0x85fc),
		        .e={0x94,0x96,0x49,0xd7,0xe4,0x5c}},
		 .symbol="efi_guid_dell",
		 .name="dell",
		 .description="Dell",
		},
		{.guid={.a=cpu_to_le32(0x7076876e),
		        .b=cpu_to_le16(0x80c2),
		        .c=cpu_to_le16(0x4ee6),
		        .d=cpu_to_be16(0xaad2),
		        .e={0x28,0xb3,0x49,0xa6,0x86,0x5b}},
		 .symbol="efi_guid_x509_sha384",
		 .name="x509_sha384",
		 .description="SHA-384 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x721c8b66),
		        .b=cpu_to_le16(0x426c),
		        .c=cpu_to_le16(0x4e86),
		        .d=cpu_to_be16(0x8e99),
		        .e={0x34,0x57,0xc4,0x6a,0xb0,0xb9}},
		 .symbol="efi_guid_lenovo_setup",
		 .name="lenovo_setup",
		 .description="Lenovo Firmware Setup",
		},
		{.guid={.a=cpu_to_le32(0x77fa9abd),
		        .b=cpu_to_le16(0x359),
		        .c=cpu_to_le16(0x4d32),
		        .d=cpu_to_be16(0xbd60),
		        .e={0x28,0xf4,0xe7,0x8f,0x78,0x4b}},
		 .symbol="efi_guid_microsoft",
		 .name="microsoft",
		 .description="Microsoft",
		},
		{.guid={.a=cpu_to_le32(0x7facc7b6),
		        .b=cpu_to_le16(0x127f),
		        .c=cpu_to_le16(0x4e9c),
		        .d=cpu_to_be16(0x9c5d),
		        .e={0x8,0xf,0x98,0x99,0x43,0x45}},
		 .symbol="efi_guid_lenovo_2",
		 .name="lenovo_2",
		 .description="Lenovo",
		},
		{.guid={.a=cpu_to_le32(0x8108ac4e),
		        .b=cpu_to_le16(0x9f11),
		        .c=cpu_to_le16(0x4d59),
		        .d=cpu_to_be16(0x850e),
		        .e={0xe2,0x1a,0x52,0x2c,0x59,0xb2}},
		 .symbol="efi_guid_auto_created_boot_option",
		 .name="auto_created_boot_option",
		 .description="Automatically Created Boot Option",
		},
		{.guid={.a=cpu_to_le32(0x826ca512),
		        .b=cpu_to_le16(0xcf10),
		        .c=cpu_to_le16(0x4ac9),
		        .d=cpu_to_be16(0xb187),
		        .e={0xbe,0x1,0x49,0x66,0x31,0xbd}},
		 .symbol="efi_guid_sha1",
		 .name="sha1",
		 .description="SHA-1",
		},
		{.guid={.a=cpu_to_le32(0x82988420),
		        .b=cpu_to_le16(0x7467),
		        .c=cpu_to_le16(0x4490),
		        .d=cpu_to_be16(0x9059),
		        .e={0xfe,0xb4,0x48,0xdd,0x19,0x63}},
		 .symbol="efi_guid_lenovo_me_config",
		 .name="lenovo_me_config",
		 .description="Lenovo ME Configuration Menu",
		},
		{.guid={.a=cpu_to_le32(0x8be4df61),
		        .b=cpu_to_le16(0x93ca),
		        .c=cpu_to_le16(0x11d2),
		        .d=cpu_to_be16(0xaa0d),
		        .e={0,0xe0,0x98,0x3,0x2b,0x8c}},
		 .symbol="efi_guid_global",
		 .name="global",
		 .description="EFI Global Variable",
		},
		{.guid={.a=cpu_to_le32(0x91376aff),
		        .b=cpu_to_le16(0xcba6),
		        .c=cpu_to_le16(0x42be),
		        .d=cpu_to_be16(0x949d),
		        .e={0x6,0xfd,0xe8,0x11,0x28,0xe8}},
		 .symbol="efi_guid_grub",
		 .name="grub",
		 .description="GRUB",
		},
		{.guid={.a=cpu_to_le32(0xa5c059a1),
		        .b=cpu_to_le16(0x94e4),
		        .c=cpu_to_le16(0x4aa7),
		        .d=cpu_to_be16(0x87b5),
		        .e={0xab,0x15,0x5c,0x2b,0xf0,0x72}},
		 .symbol="efi_guid_x509_cert",
		 .name="x509_cert",
		 .description="X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0xa7717414),
		        .b=cpu_to_le16(0xc616),
		        .c=cpu_to_le16(0x4977),
		        .d=cpu_to_be16(0x9420),
		        .e={0x84,0x47,0x12,0xa7,0x35,0xbf}},
		 .symbol="efi_guid_rsa2048_sha256_cert",
		 .name="rsa2048_sha256_cert",
		 .description="RSA-2048 key with SHA-256 Certificate",
		},
		{.guid={.a=cpu_to_le32(0xa7d8d9a6),
		        .b=cpu_to_le16(0x6ab0),
		        .c=cpu_to_le16(0x4aeb),
		        .d=cpu_to_be16(0xad9d),
		        .e={0x16,0x3e,0x59,0xa7,0xa3,0x80}},
		 .symbol="efi_guid_lenovo_diag_splash",
		 .name="lenovo_diag_splash",
		 .description="Lenovo Diagnostic Splash Screen",
		},
		{.guid={.a=cpu_to_le32(0xbc7838d2),
		        .b=cpu_to_le16(0xf82),
		        .c=cpu_to_le16(0x4d60),
		        .d=cpu_to_be16(0x8316),
		        .e={0xc0,0x68,0xee,0x79,0xd2,0x5b}},
		 .symbol="efi_guid_lenovo_msg",
		 .name="lenovo_msg",
		 .description="Lenovo Vendor Message Device",
		},
		{.guid={.a=cpu_to_le32(0xc1c41626),
		        .b=cpu_to_le16(0x504c),
		        .c=cpu_to_le16(0x4092),
		        .d=cpu_to_be16(0xaca9),
		        .e={0x41,0xf9,0x36,0x93,0x43,0x28}},
		 .symbol="efi_guid_sha256",
		 .name="sha256",
		 .description="SHA-256",
		},
		{.guid={.a=cpu_to_le32(0xc57ad6b7),
		        .b=cpu_to_le16(0x515),
		        .c=cpu_to_le16(0x40a8),
		        .d=cpu_to_be16(0x9d21),
		        .e={0x55,0x16,0x52,0x85,0x4e,0x37}},
		 .symbol="efi_guid_shell",
		 .name="shell",
		 .description="EFI Shell",
		},
		{.guid={.a=cpu_to_le32(0xd719b2cb),
		        .b=cpu_to_le16(0x3d3a),
		        .c=cpu_to_le16(0x4596),
		        .d=cpu_to_be16(0xa3bc),
		        .e={0xda,0xd0,0xe,0x67,0x65,0x6f}},
		 .symbol="efi_guid_security",
		 .name="security",
		 .description="EFI Security Database",
		},
		{.guid={.a=cpu_to_le32(0xe2b36190),
		        .b=cpu_to_le16(0x879b),
		        .c=cpu_to_le16(0x4a3d),
		        .d=cpu_to_be16(0xad8d),
		        .e={0xf2,0xe7,0xbb,0xa3,0x27,0x84}},
		 .symbol="efi_guid_rsa2048_sha256",
		 .name="rsa2048_sha256",
		 .description="RSA-2048 signature of a SHA-256 hash",
		},
		{.guid={.a=cpu_to_le32(0xf46ee6f4),
		        .b=cpu_to_le16(0x4785),
		        .c=cpu_to_le16(0x43a3),
		        .d=cpu_to_be16(0x923d),
		        .e={0x7f,0x78,0x6c,0x3c,0x84,0x79}},
		 .symbol="efi_guid_lenovo_startup_interrupt",
		 .name="lenovo_startup_interrupt",
		 .description="Lenovo Startup Interrupt Menu",
		},
		{.guid={.a=cpu_to_le32(0xff3e5307),
		        .b=cpu_to_le16(0x9fd0),
		        .c=cpu_to_le16(0x48c9),
		        .d=cpu_to_be16(0x85f1),
		        .e={0x8a,0xd5,0x6c,0x70,0x1e,0x1}},
		 .symbol="efi_guid_sha384",
		 .name="sha384",
		 .description="SHA-384",
		},
		{.guid={.a=cpu_to_le32(0xffffffff),
		        .b=cpu_to_le16(0xffff),
		        .c=cpu_to_le16(0xffff),
		        .d=cpu_to_be16(0xffff),
		        .e={0xff,0xff,0xff,0xff,0xff,0xff}},
		 .symbol="efi_guid_zzignore-this-guid",
		 .name="zzignore-this-guid",
		 .description="zzignore-this-guid",
		},
};
const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_names_[42]= {
		{.guid={.a=cpu_to_le32(0x3b053091),
		        .b=cpu_to_le16(0x6c9f),
		        .c=cpu_to_le16(0x4cc),
		        .d=cpu_to_be16(0xb1ac),
		        .e={0xe2,0xa5,0x1e,0x3b,0xe5,0xf5}},
		 .symbol="efi_guid_asus",
		 .name="asus",
		 .description="Asus",
		},
		{.guid={.a=cpu_to_le32(0x8108ac4e),
		        .b=cpu_to_le16(0x9f11),
		        .c=cpu_to_le16(0x4d59),
		        .d=cpu_to_be16(0x850e),
		        .e={0xe2,0x1a,0x52,0x2c,0x59,0xb2}},
		 .symbol="efi_guid_auto_created_boot_option",
		 .name="auto_created_boot_option",
		 .description="Automatically Created Boot Option",
		},
		{.guid={.a=cpu_to_le32(0x6dc40ae4),
		        .b=cpu_to_le16(0x2ee8),
		        .c=cpu_to_le16(0x9c4c),
		        .d=cpu_to_be16(0xa314),
		        .e={0xf,0xc7,0xb2,0,0x87,0x10}},
		 .symbol="efi_guid_canonical",
		 .name="canonical",
		 .description="Canonical",
		},
		{.guid={.a=cpu_to_le32(0x70564dce),
		        .b=cpu_to_le16(0x9afc),
		        .c=cpu_to_le16(0x4ee3),
		        .d=cpu_to_be16(0x85fc),
		        .e={0x94,0x96,0x49,0xd7,0xe4,0x5c}},
		 .symbol="efi_guid_dell",
		 .name="dell",
		 .description="Dell",
		},
		{.guid={.a=cpu_to_le32(0x452e8ced),
		        .b=cpu_to_le16(0xdfff),
		        .c=cpu_to_le16(0x4b8c),
		        .d=cpu_to_be16(0xae01),
		        .e={0x51,0x18,0x86,0x2e,0x68,0x2c}},
		 .symbol="efi_guid_external_management",
		 .name="external_management",
		 .description="External Management Mechanism",
		},
		{.guid={.a=cpu_to_le32(0x55555555),
		        .b=cpu_to_le16(0x5555),
		        .c=cpu_to_le16(0x5555),
		        .d=cpu_to_be16(0x5555),
		        .e={0x55,0x55,0x55,0x55,0x55,0x55}},
		 .symbol="efi_guid_fives",
		 .name="fives",
		 .description="All Fives Test Data",
		},
		{.guid={.a=cpu_to_le32(0xabba7dc),
		        .b=cpu_to_le16(0xe516),
		        .c=cpu_to_le16(0x4167),
		        .d=cpu_to_be16(0xbbf5),
		        .e={0x4d,0x9d,0x1c,0x73,0x94,0x16}},
		 .symbol="efi_guid_fwupdate",
		 .name="fwupdate",
		 .description="Linux Firmware Update Tool",
		},
		{.guid={.a=cpu_to_le32(0x8be4df61),
		        .b=cpu_to_le16(0x93ca),
		        .c=cpu_to_le16(0x11d2),
		        .d=cpu_to_be16(0xaa0d),
		        .e={0,0xe0,0x98,0x3,0x2b,0x8c}},
		 .symbol="efi_guid_global",
		 .name="global",
		 .description="EFI Global Variable",
		},
		{.guid={.a=cpu_to_le32(0x91376aff),
		        .b=cpu_to_le16(0xcba6),
		        .c=cpu_to_le16(0x42be),
		        .d=cpu_to_be16(0x949d),
		        .e={0x6,0xfd,0xe8,0x11,0x28,0xe8}},
		 .symbol="efi_guid_grub",
		 .name="grub",
		 .description="GRUB",
		},
		{.guid={.a=cpu_to_le32(0x3cc24e96),
		        .b=cpu_to_le16(0x22c7),
		        .c=cpu_to_le16(0x41d8),
		        .d=cpu_to_be16(0x8863),
		        .e={0x8e,0x39,0xdc,0xdc,0xc2,0xcf}},
		 .symbol="efi_guid_lenovo",
		 .name="lenovo",
		 .description="Lenovo",
		},
		{.guid={.a=cpu_to_le32(0x7facc7b6),
		        .b=cpu_to_le16(0x127f),
		        .c=cpu_to_le16(0x4e9c),
		        .d=cpu_to_be16(0x9c5d),
		        .e={0x8,0xf,0x98,0x99,0x43,0x45}},
		 .symbol="efi_guid_lenovo_2",
		 .name="lenovo_2",
		 .description="Lenovo",
		},
		{.guid={.a=cpu_to_le32(0x126a762d),
		        .b=cpu_to_le16(0x5758),
		        .c=cpu_to_le16(0x4fca),
		        .d=cpu_to_be16(0x8531),
		        .e={0x20,0x1a,0x7f,0x57,0xf8,0x50}},
		 .symbol="efi_guid_lenovo_boot_menu",
		 .name="lenovo_boot_menu",
		 .description="Lenovo Boot Menu",
		},
		{.guid={.a=cpu_to_le32(0x3f7e615b),
		        .b=cpu_to_le16(0xd45),
		        .c=cpu_to_le16(0x4f80),
		        .d=cpu_to_be16(0x88dc),
		        .e={0x26,0xb2,0x34,0x95,0x85,0x60}},
		 .symbol="efi_guid_lenovo_diag",
		 .name="lenovo_diag",
		 .description="Lenovo Diagnostics",
		},
		{.guid={.a=cpu_to_le32(0xa7d8d9a6),
		        .b=cpu_to_le16(0x6ab0),
		        .c=cpu_to_le16(0x4aeb),
		        .d=cpu_to_be16(0xad9d),
		        .e={0x16,0x3e,0x59,0xa7,0xa3,0x80}},
		 .symbol="efi_guid_lenovo_diag_splash",
		 .name="lenovo_diag_splash",
		 .description="Lenovo Diagnostic Splash Screen",
		},
		{.guid={.a=cpu_to_le32(0x82988420),
		        .b=cpu_to_le16(0x7467),
		        .c=cpu_to_le16(0x4490),
		        .d=cpu_to_be16(0x9059),
		        .e={0xfe,0xb4,0x48,0xdd,0x19,0x63}},
		 .symbol="efi_guid_lenovo_me_config",
		 .name="lenovo_me_config",
		 .description="Lenovo ME Configuration Menu",
		},
		{.guid={.a=cpu_to_le32(0xbc7838d2),
		        .b=cpu_to_le16(0xf82),
		        .c=cpu_to_le16(0x4d60),
		        .d=cpu_to_be16(0x8316),
		        .e={0xc0,0x68,0xee,0x79,0xd2,0x5b}},
		 .symbol="efi_guid_lenovo_msg",
		 .name="lenovo_msg",
		 .description="Lenovo Vendor Message Device",
		},
		{.guid={.a=cpu_to_le32(0x665d3f60),
		        .b=cpu_to_le16(0xad3e),
		        .c=cpu_to_le16(0x4cad),
		        .d=cpu_to_be16(0x8e26),
		        .e={0xdb,0x46,0xee,0xe9,0xf1,0xb5}},
		 .symbol="efi_guid_lenovo_rescue",
		 .name="lenovo_rescue",
		 .description="Lenovo Rescue and Recovery",
		},
		{.guid={.a=cpu_to_le32(0x721c8b66),
		        .b=cpu_to_le16(0x426c),
		        .c=cpu_to_le16(0x4e86),
		        .d=cpu_to_be16(0x8e99),
		        .e={0x34,0x57,0xc4,0x6a,0xb0,0xb9}},
		 .symbol="efi_guid_lenovo_setup",
		 .name="lenovo_setup",
		 .description="Lenovo Firmware Setup",
		},
		{.guid={.a=cpu_to_le32(0xf46ee6f4),
		        .b=cpu_to_le16(0x4785),
		        .c=cpu_to_le16(0x43a3),
		        .d=cpu_to_be16(0x923d),
		        .e={0x7f,0x78,0x6c,0x3c,0x84,0x79}},
		 .symbol="efi_guid_lenovo_startup_interrupt",
		 .name="lenovo_startup_interrupt",
		 .description="Lenovo Startup Interrupt Menu",
		},
		{.guid={.a=cpu_to_le32(0x77fa9abd),
		        .b=cpu_to_le16(0x359),
		        .c=cpu_to_le16(0x4d32),
		        .d=cpu_to_be16(0xbd60),
		        .e={0x28,0xf4,0xe7,0x8f,0x78,0x4b}},
		 .symbol="efi_guid_microsoft",
		 .name="microsoft",
		 .description="Microsoft",
		},
		{.guid={.a=cpu_to_le32(0x4aafd29d),
		        .b=cpu_to_le16(0x68df),
		        .c=cpu_to_le16(0x49ee),
		        .d=cpu_to_be16(0x8aa9),
		        .e={0x34,0x7d,0x37,0x56,0x65,0xa7}},
		 .symbol="efi_guid_pkcs7_cert",
		 .name="pkcs7_cert",
		 .description="PKCS7 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x223eddb),
		        .b=cpu_to_le16(0x9079),
		        .c=cpu_to_le16(0x4388),
		        .d=cpu_to_be16(0xaf77),
		        .e={0x2d,0x65,0xb1,0xc3,0x5d,0x3b}},
		 .symbol="efi_guid_redhat",
		 .name="redhat",
		 .description="Red Hat",
		},
		{.guid={.a=cpu_to_le32(0x3c5766e8),
		        .b=cpu_to_le16(0x269c),
		        .c=cpu_to_le16(0x4e34),
		        .d=cpu_to_be16(0xaa14),
		        .e={0xed,0x77,0x6e,0x85,0xb3,0xb6}},
		 .symbol="efi_guid_rsa2048",
		 .name="rsa2048",
		 .description="RSA 2048 pubkey",
		},
		{.guid={.a=cpu_to_le32(0x67f8444f),
		        .b=cpu_to_le16(0x8743),
		        .c=cpu_to_le16(0x48f1),
		        .d=cpu_to_be16(0xa328),
		        .e={0x1e,0xaa,0xb8,0x73,0x60,0x80}},
		 .symbol="efi_guid_rsa2048_sha1",
		 .name="rsa2048_sha1",
		 .description="RSA-2048 signature of a SHA-1 hash",
		},
		{.guid={.a=cpu_to_le32(0xe2b36190),
		        .b=cpu_to_le16(0x879b),
		        .c=cpu_to_le16(0x4a3d),
		        .d=cpu_to_be16(0xad8d),
		        .e={0xf2,0xe7,0xbb,0xa3,0x27,0x84}},
		 .symbol="efi_guid_rsa2048_sha256",
		 .name="rsa2048_sha256",
		 .description="RSA-2048 signature of a SHA-256 hash",
		},
		{.guid={.a=cpu_to_le32(0xa7717414),
		        .b=cpu_to_le16(0xc616),
		        .c=cpu_to_le16(0x4977),
		        .d=cpu_to_be16(0x9420),
		        .e={0x84,0x47,0x12,0xa7,0x35,0xbf}},
		 .symbol="efi_guid_rsa2048_sha256_cert",
		 .name="rsa2048_sha256_cert",
		 .description="RSA-2048 key with SHA-256 Certificate",
		},
		{.guid={.a=cpu_to_le32(0xd719b2cb),
		        .b=cpu_to_le16(0x3d3a),
		        .c=cpu_to_le16(0x4596),
		        .d=cpu_to_be16(0xa3bc),
		        .e={0xda,0xd0,0xe,0x67,0x65,0x6f}},
		 .symbol="efi_guid_security",
		 .name="security",
		 .description="EFI Security Database",
		},
		{.guid={.a=cpu_to_le32(0x826ca512),
		        .b=cpu_to_le16(0xcf10),
		        .c=cpu_to_le16(0x4ac9),
		        .d=cpu_to_be16(0xb187),
		        .e={0xbe,0x1,0x49,0x66,0x31,0xbd}},
		 .symbol="efi_guid_sha1",
		 .name="sha1",
		 .description="SHA-1",
		},
		{.guid={.a=cpu_to_le32(0xb6e5233),
		        .b=cpu_to_le16(0xa65c),
		        .c=cpu_to_le16(0x44c9),
		        .d=cpu_to_be16(0x9407),
		        .e={0xd9,0xab,0x83,0xbf,0xc8,0xbd}},
		 .symbol="efi_guid_sha224",
		 .name="sha224",
		 .description="SHA-224 hash",
		},
		{.guid={.a=cpu_to_le32(0xc1c41626),
		        .b=cpu_to_le16(0x504c),
		        .c=cpu_to_le16(0x4092),
		        .d=cpu_to_be16(0xaca9),
		        .e={0x41,0xf9,0x36,0x93,0x43,0x28}},
		 .symbol="efi_guid_sha256",
		 .name="sha256",
		 .description="SHA-256",
		},
		{.guid={.a=cpu_to_le32(0xff3e5307),
		        .b=cpu_to_le16(0x9fd0),
		        .c=cpu_to_le16(0x48c9),
		        .d=cpu_to_be16(0x85f1),
		        .e={0x8a,0xd5,0x6c,0x70,0x1e,0x1}},
		 .symbol="efi_guid_sha384",
		 .name="sha384",
		 .description="SHA-384",
		},
		{.guid={.a=cpu_to_le32(0x93e0fae),
		        .b=cpu_to_le16(0xa6c4),
		        .c=cpu_to_le16(0x4f50),
		        .d=cpu_to_be16(0x9f1b),
		        .e={0xd4,0x1e,0x2b,0x89,0xc1,0x9a}},
		 .symbol="efi_guid_sha512",
		 .name="sha512",
		 .description="SHA-512 hash",
		},
		{.guid={.a=cpu_to_le32(0xc57ad6b7),
		        .b=cpu_to_le16(0x515),
		        .c=cpu_to_le16(0x40a8),
		        .d=cpu_to_be16(0x9d21),
		        .e={0x55,0x16,0x52,0x85,0x4e,0x37}},
		 .symbol="efi_guid_shell",
		 .name="shell",
		 .description="EFI Shell",
		},
		{.guid={.a=cpu_to_le32(0x605dab50),
		        .b=cpu_to_le16(0xe046),
		        .c=cpu_to_le16(0x4300),
		        .d=cpu_to_be16(0xabb6),
		        .e={0x3d,0xd8,0x10,0xdd,0x8b,0x23}},
		 .symbol="efi_guid_shim",
		 .name="shim",
		 .description="shim",
		},
		{.guid={.a=cpu_to_le32(0x26dc4851),
		        .b=cpu_to_le16(0x195f),
		        .c=cpu_to_le16(0x4ae1),
		        .d=cpu_to_be16(0x9a19),
		        .e={0xfb,0xf8,0x83,0xbb,0xb3,0x5e}},
		 .symbol="efi_guid_supermicro",
		 .name="supermicro",
		 .description="Super Micro",
		},
		{.guid={.a=cpu_to_le32(0x3b8c8162),
		        .b=cpu_to_le16(0x188c),
		        .c=cpu_to_le16(0x46a4),
		        .d=cpu_to_be16(0xaec9),
		        .e={0xbe,0x43,0xf1,0xd6,0x56,0x97}},
		 .symbol="efi_guid_ux_capsule",
		 .name="ux_capsule",
		 .description="Firmware update localized text image",
		},
		{.guid={.a=cpu_to_le32(0xa5c059a1),
		        .b=cpu_to_le16(0x94e4),
		        .c=cpu_to_le16(0x4aa7),
		        .d=cpu_to_be16(0x87b5),
		        .e={0xab,0x15,0x5c,0x2b,0xf0,0x72}},
		 .symbol="efi_guid_x509_cert",
		 .name="x509_cert",
		 .description="X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x3bd2a492),
		        .b=cpu_to_le16(0x96c0),
		        .c=cpu_to_le16(0x4079),
		        .d=cpu_to_be16(0xb420),
		        .e={0xfc,0xf9,0x8e,0xf1,0x3,0xed}},
		 .symbol="efi_guid_x509_sha256",
		 .name="x509_sha256",
		 .description="SHA-256 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x7076876e),
		        .b=cpu_to_le16(0x80c2),
		        .c=cpu_to_le16(0x4ee6),
		        .d=cpu_to_be16(0xaad2),
		        .e={0x28,0xb3,0x49,0xa6,0x86,0x5b}},
		 .symbol="efi_guid_x509_sha384",
		 .name="x509_sha384",
		 .description="SHA-384 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0x446dbf63),
		        .b=cpu_to_le16(0x2502),
		        .c=cpu_to_le16(0x4cda),
		        .d=cpu_to_be16(0xbcfa),
		        .e={0x24,0x65,0xd2,0xb0,0xfe,0x9d}},
		 .symbol="efi_guid_x509_sha512",
		 .name="x509_sha512",
		 .description="SHA-512 hash of X.509 Certificate",
		},
		{.guid={.a=cpu_to_le32(0),
		        .b=cpu_to_le16(0),
		        .c=cpu_to_le16(0),
		        .d=cpu_to_be16(0x0000),
		        .e={0,0,0,0,0,0}},
		 .symbol="efi_guid_zero",
		 .name="zero",
		 .description="zeroed sentinel guid",
		},
		{.guid={.a=cpu_to_le32(0xffffffff),
		        .b=cpu_to_le16(0xffff),
		        .c=cpu_to_le16(0xffff),
		        .d=cpu_to_be16(0xffff),
		        .e={0xff,0xff,0xff,0xff,0xff,0xff}},
		 .symbol="efi_guid_zzignore-this-guid",
		 .name="zzignore-this-guid",
		 .description="zzignore-this-guid",
		},
};
