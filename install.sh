#!/bin/bash

# ========== CONFIGURATION ==========
ISO="./RainFall.iso"
RAM=1024
CPUS=2
SSH_PORT=4242
# ===================================

if [ ! -f "$ISO" ]; then
    echo "Erreur : ISO introuvable → $ISO"
    exit 1
fi

if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Erreur : QEMU n'est pas installé"
    echo "→ sudo apt install qemu-system-x86"
    exit 1
fi

echo "[+] Lancement de RainFall (mode headless)..."
echo "[+] SSH : ssh level0@127.0.0.1 -p $SSH_PORT"
echo "[+] Mot de passe : level0"
echo "[+] Pour quitter la VM : Ctrl+A puis X"
echo "[+] (aide QEMU : Ctrl+A puis H)"
echo ""

# Essaye d'abord avec KVM (beaucoup plus rapide), sinon sans
qemu-system-x86_64 \
    -m $RAM \
    -smp $CPUS \
    -cdrom "$ISO" \
    -boot d \
    -netdev user,id=net0,hostfwd=tcp::${SSH_PORT}-:4242 \
    -device e1000,netdev=net0 \
    -nographic \
    -enable-kvm 2>/dev/null || \
qemu-system-x86_64 \
    -m $RAM \
    -smp $CPUS \
    -cdrom "$ISO" \
    -boot d \
    -netdev user,id=net0,hostfwd=tcp::${SSH_PORT}-:4242 \
    -device e1000,netdev=net0 \
    -nographic
