import secrets

def process_huella():
    # Generar 512 bytes aleatorios
    random_bytes = secrets.token_bytes(512)
    # Convertir a cadena hexadecimal
    hex_string = random_bytes.hex()
    return hex_string
