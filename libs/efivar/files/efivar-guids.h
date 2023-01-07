#ifndef EFIVAR_GUIDS_H
#define EFIVAR_GUIDS_H 1


struct efivar_guidname {
	efi_guid_t guid;
	char symbol[256];
	char name[256];
	char description[256];
} __attribute__((__aligned__(16)));

extern const efi_guid_t efi_guid_empty __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_zero __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_redhat_2 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_redhat __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_sha512 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_fwupdate __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_sha224 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_boot_menu __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_supermicro __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_asus __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_ux_capsule __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_x509_sha256 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_rsa2048 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_diag __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_x509_sha512 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_external_management __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_pkcs7_cert __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_fives __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_shim __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_rescue __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_rsa2048_sha1 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_canonical __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_dell __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_x509_sha384 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_setup __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_microsoft __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_2 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_auto_created_boot_option __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_sha1 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_me_config __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_global __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_grub __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_x509_cert __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_rsa2048_sha256_cert __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_diag_splash __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_msg __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_sha256 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_shell __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_security __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_rsa2048_sha256 __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_lenovo_startup_interrupt __attribute__((__visibility__ ("default")));
extern const efi_guid_t efi_guid_sha384 __attribute__((__visibility__ ("default")));

#ifndef EFIVAR_BUILD_ENVIRONMENT

extern const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_guids[41];
extern const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_guids_end;
extern const uint64_t
	__attribute__((__visibility__ ("default")))
	efi_n_well_known_guids;

extern const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_names[41];
extern const struct efivar_guidname
	__attribute__((__visibility__ ("default")))
	efi_well_known_names_end;
extern const uint64_t
	__attribute__((__visibility__ ("default")))
	efi_n_well_known_names;

#endif /* EFIVAR_BUILD_ENVIRONMENT */

#endif /* EFIVAR_GUIDS_H */
