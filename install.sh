#!/bin/bash
set -euo pipefail

# ========== CONFIGURATION ==========
ISO="./iso/RainFall.iso"
RAM=1024
CPUS=2
SSH_PORT=4242
SHARE_ENABLE=0            # 1 = partager ./share avec la VM (9p/virtfs)
SHARE_DIR="./share"
SHARE_TAG="host0"
# ===================================

if [ ! -f "$ISO" ]; then
    echo "Erreur : ISO introuvable -> $ISO"
    exit 1
fi

if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Erreur : QEMU n'est pas installe"
    case "$(uname -s)" in
        Darwin) echo "-> brew install qemu" ;;
        *)      echo "-> sudo apt install qemu-system-x86" ;;
    esac
    exit 1
fi

# Choix de l'accelerateur selon l'hote.
# ARM (Apple Silicon) executant un invite x86 => emulation logicielle (TCG),
# aucune accel materielle possible (HVF n'accelere que x86-sur-x86).
ACCEL="tcg,thread=multi"
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)  [ -e /dev/kvm ] && ACCEL="kvm" ;;
    Darwin-x86_64) ACCEL="hvf" ;;
    Darwin-arm64)  ACCEL="tcg,thread=multi" ;;
esac

# Arguments QEMU.
ARGS=(
    -accel "${ACCEL}"
    -m "$RAM"
    -smp "$CPUS"
    -cdrom "$ISO"
    -boot d
    -netdev "user,id=net0,hostfwd=tcp::${SSH_PORT}-:4242"
    -device e1000,netdev=net0
    -nographic
)

# Partage de dossier optionnel (evite les scp repetes).
if [ "$SHARE_ENABLE" = "1" ]; then
    mkdir -p "$SHARE_DIR"
    ARGS+=( -virtfs "local,path=${SHARE_DIR},mount_tag=${SHARE_TAG},security_model=mapped-xattr,id=${SHARE_TAG}" )
fi

echo "[+] Lancement de RainFall (headless, accel=${ACCEL})..."
echo "[+] SSH : ssh level0@127.0.0.1 -p $SSH_PORT   (mot de passe : level0)"
echo "[+] Copier un binaire : scp -P $SSH_PORT level0@127.0.0.1:/home/user/levelN/levelN ./bin/"
if [ "$SHARE_ENABLE" = "1" ]; then
    echo "[+] Dossier partage : ${SHARE_DIR} -> dans la VM : sudo mount -t 9p -o trans=virtio ${SHARE_TAG} /mnt"
fi
echo "[+] Quitter la VM : Ctrl+A puis X   (aide QEMU : Ctrl+A puis H)"
echo ""

exec qemu-system-x86_64 "${ARGS[@]}"
