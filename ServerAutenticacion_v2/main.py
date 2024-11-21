from fastapi import FastAPI, BackgroundTasks, HTTPException
from pydantic import BaseModel
from pymongo import MongoClient
import secrets
from datetime import datetime
import pytz 
import os
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from base64 import b64encode

app = FastAPI()

# Conexion a base de datos MongoDB
client = MongoClient('mongodb+srv://luortiz:321@cluster0.n3xcu.mongodb.net/')
db = client['bioaccess']
collection_huellas = db['huellas']
collection_accesos = db["acceso"]
collection_porterias = db["porteria"]
collection_usuarios = db["usuario"]
# ------------ REGISTRO --------------------
# Clave y vector de inicialización para AES
CLAVE_AES = os.urandom(32)  # Genera una clave de 256 bits
IV = os.urandom(16)         # Genera un IV de 128 bits

# ------------ REGISTRO --------------------
# Modelo estructura de registro
class TemplateData(BaseModel):
    cedula: str
    id: str

def cifrar_datos(data, clave, iv):
    """Cifra los datos usando AES en modo CBC."""
    # Aplicar padding para manejar datos que no sean múltiplos del bloque
    padder = padding.PKCS7(128).padder()
    data_padded = padder.update(data) + padder.finalize()

    # Cifrar datos
    cipher = Cipher(algorithms.AES(clave), modes.CBC(iv))
    encryptor = cipher.encryptor()
    return encryptor.update(data_padded) + encryptor.finalize()

def process_huella():
    """Genera tres templates de huella."""
    base_bytes = bytearray(secrets.token_bytes(512))
    
    # Crear variaciones para los otros dos templates
    template1 = base_bytes.hex()
    
    # Hacer una copia y modificar ligeramente algunos bytes
    template2_bytes = bytearray(base_bytes)
    for i in range(5):  # Modificar 5 bytes en posiciones aleatorias
        pos = secrets.randbelow(len(template2_bytes))
        template2_bytes[pos] ^= secrets.randbelow(256)
    
    template2 = template2_bytes.hex()
    
    # Crear una tercera variación
    template3_bytes = bytearray(base_bytes)
    for i in range(5):  # Modificar 5 bytes en posiciones aleatorias
        pos = secrets.randbelow(len(template3_bytes))
        template3_bytes[pos] ^= secrets.randbelow(256)
    
    template3 = template3_bytes.hex()
    
    return [template1, template2, template3]

@app.post("/registrar_usuario/")
async def registrar_usuario(data: TemplateData):
    cedula = data.cedula
    huella_vector = process_huella()
    
    # Cifrar cada template antes de almacenarlo
    templates_cifrados = []
    for template in huella_vector:
        template_bytes = bytes.fromhex(template)  # Convertir a bytes
        encrypted = cifrar_datos(template_bytes, CLAVE_AES, IV)
        templates_cifrados.append(b64encode(encrypted).decode('utf-8'))  # Codificar en base64
    # Guardar en la base de datos
    registro = {
        "_id": cedula,
        "templates": templates_cifrados,
        "iv": b64encode(IV).decode('utf-8')  # Guardar IV para descifrar posteriormente
    }
    collection_huellas.insert_one(registro)
    return {"message": f"Registro Exitoso: CC: {cedula}"}

# ------------ AUTENTICACION ---------------
# Modelo para la estructura del JSON recibido
class AccessData(BaseModel):
    id: str
    access: str
    mac: str

@app.post("/verificar_acceso/")
async def verificar_acceso(
    background_tasks: BackgroundTasks,  # Tareas en segundo plano (Crear registro de acceso)
    data: AccessData
):
    # Identificar porteria con MAC
    porteria = collection_porterias.find_one({"mac":data.mac})
    if porteria:
        # Convertir el _id de la portería a int32
        porteria_id = int(porteria["_id"]) 
    else:
        print("Porteria no encontrada")
        # Si no se encuentra la mac asociada a la porteria
        raise HTTPException(status_code=403, detail="Punto de acceso (porteria) no encontrada")  # Retorna 403 Forbidden

    # Verificar el valor de "access" en el JSON
    print(data)
    if data.access == "1":
        # Registrar acceso exitoso en la base de datos
        # Obtener cedula autenticada
        # Asociar cedula a id
        inc = int(data.id)
        usuario_auth = collection_usuarios.find_one({"in":inc})
        cedula_auth = str(usuario_auth["_id"]) 
        print("Usuario autenticado: " + usuario_auth["nombre"])
        print(cedula_auth)
        background_tasks.add_task(registrar_acceso, cedula_auth, porteria_id, True)
        # Imprimir información del usuario
        print("Usuario autenticado : " + usuario_auth["nombre"])
        return {"message": "Acceso concedido"}  # Respuesta 200 OK por defecto en FastAPI
    else:
        # Registrar acceso fallido
        print("Acceso denegado")
        background_tasks.add_task(registrar_acceso, None, porteria_id, False)
        raise HTTPException(status_code=403, detail="Acceso denegado")  # Retorna 403 Forbidden


# Funciones
# Función asíncrona para registrar el acceso
now=datetime.now(pytz.timezone('America/Bogota'))
Formatted_time = now.strftime('%a %b %d %H:%M:%S COT %Y')
print(Formatted_time)
async def registrar_acceso(cedula, porteria, autenticado):
    # Fecha y hora actual
    fecha_acceso_form = datetime.now(pytz.timezone('America/Bogota'))
    fecha_acceso = fecha_acceso_form.strftime('%a %b %d %H:%M:%S COT %Y')
    # Si la autenticación falló, la cédula es None
    if not autenticado:
        cedula = None
    # Crear el registro de acceso
    registro_acceso = {
        "cedula": cedula,
        "porteria": porteria,
        "fecha_hora": fecha_acceso,
        "autenticado": autenticado
    }
    # Insertar el registro en la colección "accesos"
    collection_accesos.insert_one(registro_acceso)

# Funcion para procesar huella
def process_huella():
    # Generar el primer template aleatorio de 512 bytes
    base_bytes = bytearray(secrets.token_bytes(512))
    
    # Crear variaciones para los otros dos templates
    template1 = base_bytes.hex()
    
    # Hacer una copia y modificar ligeramente algunos bytes
    template2_bytes = bytearray(base_bytes)
    for i in range(5):  # Modificar 5 bytes en posiciones aleatorias
        pos = secrets.randbelow(len(template2_bytes))
        template2_bytes[pos] ^= secrets.randbelow(256)  # XOR aleatorio con un valor de 0 a 255
    
    template2 = template2_bytes.hex()
    
    # Crear una tercera variación
    template3_bytes = bytearray(base_bytes)
    for i in range(5):  # Modificar 5 bytes en posiciones aleatorias
        pos = secrets.randbelow(len(template3_bytes))
        template3_bytes[pos] ^= secrets.randbelow(256)  # XOR aleatorio con un valor de 0 a 255
    
    template3 = template3_bytes.hex()
    
    return [template1, template2, template3]
