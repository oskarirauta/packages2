SECTIONS
{
  .data :
  {
    efi_well_known_guids = efi_well_known_guids_;
    efi_well_known_guids_end = efi_well_known_guids_ + 32144;
    efi_well_known_names = efi_well_known_names_;
    efi_well_known_names_end = efi_well_known_names_ + 32144;
  }
} INSERT AFTER .data;
