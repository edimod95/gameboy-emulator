# make_boot.py
# ROZBUDOWANA WERSJA BOOT ROM-U
boot_rom_bytes = bytearray([
    0x31, 0xFE, 0xFF,  # 0x0000: LD SP,$fffe
    0xAF,              # 0x0003: XOR A
    0x21, 0xFF, 0x9F,  # 0x0004: LD HL,$9fff
    0x32,              # 0x0007: LD (HL-),A
    0xCB, 0x7C,        # 0x0008: BIT 7,H
    0x20, 0xFB,        # 0x000a: JR NZ, Addr_0007
    
    0x21, 0x26, 0xFF,  # 0x000c: LD HL,$ff26
    0x0E, 0x11,        # 0x000f: LD C,$11
    0x3E, 0x80,        # 0x0011: LD A,$80
    0x32,              # 0x0013: LD (HL-),A
    0xE2,              # 0x0014: LD ($FF00+C),A
    0x0C,              # 0x0015: INC C
    0x3E, 0xF3,        # 0x0016: LD A,$f3
    0xE2,              # 0x0018: LD ($FF00+C),A
    0x32,              # 0x0019: LD (HL-),A
    0x3E, 0x77,        # 0x001a: LD A,$77
    0x77,              # 0x001c: LD (HL),A
    
    0x3E, 0xFC,        # 0x001d: LD A,$fc
    0xE0, 0x47,        # 0x001f: LD ($FF00+$47),A
    
    0x11, 0x04, 0x01,  # 0x0021: LD DE,$0104
    0x21, 0x10, 0x80,  # 0x0024: LD HL,$8010
    
    # --- NOWA SEKCYJNA OD ADRESU 0x0027 ---
    0x1A,              # 0x0027: LD A,(DE)      <- Tutaj wcześniej było 0x42!
    0xCD, 0x95, 0x00,  # 0x0028: CALL $0095
    0xCD, 0x96, 0x00,  # 0x002b: CALL $0096     (Na razie skoczy w to samo miejsce)
    0x13,              # 0x002e: INC DE
    0x7B,              # 0x002f: LD A,E
    0xFE, 0x34,        # 0x0030: CP $34
    0x20, 0xF2,        # 0x0032: JR NZ, Addr_0027 (skok relatywny o -14 bajtów w tył)
    
    # Nasz nowy bezpiecznik na adresie 0x0034
    0x42               # 0x0034: Nowy koniec programu
])

# Ponieważ program wykonuje skok pod adres 0x0095, musimy wypełnić pamięć 
# pomiędzy adresem 0x0035 a 0x0094 zerami (NOPami), a pod 0x0095 wstawić procedurę graficzną!
while len(boot_rom_bytes) < 0x0095:
    boot_rom_bytes.append(0x00) # Wypełniamy puste miejsce NOPami

# Dopasowujemy procedurę graficzną od adresu 0x0095:
boot_rom_bytes.extend([
    0x4F,              # 0x0095: LD C,A
    0x06, 0x04,        # 0x0096: LD B,$04
    # Pętla Addr_0098:
    0xC5,              # 0x0098: PUSH BC
    0xCB, 0x11,        # 0x0099: RL C
    0x17,              # 0x009b: RLA
    0xC1,              # 0x009c: POP BC
    0xCB, 0x11,        # 0x009d: RL C
    0x17,              # 0x009f: RLA
    0x05,              # 0x00a0: DEC B          (Musimy to zaraz dopisać do cpu.c!)
    0x20, 0xF5,        # 0x00a1: JR NZ, Addr_0098
    
    0x42               # Nowy bezpiecznik wewnątrz procedury graficznej
])

with open("dmg_boot.bin", "wb") as f:
    f.write(boot_rom_bytes)

print(f"Plik dmg_boot.bin został rozbudowany! Aktualny rozmiar: {len(boot_rom_bytes)} bajtów.")
