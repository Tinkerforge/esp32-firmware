# Install libsodium
sudo apt install libsodium23

# Install libccid
sudo apt install libccid

# Install gpg
sudo apt install gpg

# Install keepassxc
sudo apt install keepassxc

# Setup Nitrokey for usage with KeePassXC
https://docs.nitrokey.com/de/software/nk-app2/keepassxc

# Create config.ini based on config.ini.example

# Open KeePassXC and create sodium_secret_key_v1.kdbx and gpg_keyring_passphrase_v1.kdbx databases.
# Configure the choosen protection (token, keyfile or password) in config.ini

# Run keygen.py once for each preset configured in config.ini to generate the keys
python keygen.py <preset>
