from fastapi import FastAPI, BackgroundTasks, HTTPException, Request
from pydantic import BaseModel
from pymongo import MongoClient
import secrets
from datetime import datetime
app = FastAPI()

# Conexion a base de datos MongoDB
client = MongoClient('mongodb+srv://luortiz:321@cluster0.n3xcu.mongodb.net/')
db = client['bioaccess']
collection_huellas = db['huellas']
collection_accesos = db["acceso"]
collection_porterias = db["porteria"]
collection_usuarios = db["usuario"]
# ------------ REGISTRO --------------------
# Modelo estructura de registro
class TemplateData (BaseModel):
    cedula: str
    id: str

@app.post("/registrar_usuario/")
async def registrar_usuario(data: TemplateData):
    cedula = data.cedula
    huella_vector = process_huella()
    print(huella_vector)
    # Guardar huella en la base de datos
    registro = {
        "_id": cedula,
        "templates":huella_vector,  # Guardamos los vectores directamente
    }
    collection_huellas.insert_one(registro)
    return {"message": "Registro Exitoso"} # Respuesta de 200 OK

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
    # Asociar cedula a id
    inc = int(data.id)
    # Identificar porteria con MAC
    porteria = collection_porterias.find_one({"mac":data.mac})
    if porteria:
        # Convertir el _id de la portería a int32
        porteria_id = int(porteria["_id"]) 
    else:
        # Si no se encuentra la mac asociada a la porteria
        raise HTTPException(status_code=403, detail="Punto de acceso (porteria) no encontrada")  # Retorna 403 Forbidden

    # Verificar el valor de "access" en el JSON
    print(data)
    if data.access == "1":
        # Registrar acceso exitoso en la base de datos
        # Obtener cedula autenticada
        usuario_auth = collection_usuarios.find_one({"in":inc})
        cedula_auth = usuario_auth["_id"]
        background_tasks.add_task(registrar_acceso, cedula_auth, porteria_id, True)
        # Imprimir información del usuario
        print("Usuario autenticado : " + usuario_auth["nombre"])

        return {"message": "Acceso concedido"}  # Respuesta 200 OK por defecto en FastAPI
    else:
        # Registrar acceso fallido
        background_tasks.add_task(registrar_acceso, None, porteria_id, False)
        raise HTTPException(status_code=403, detail="Acceso denegado")  # Retorna 403 Forbidden


# Funciones
# Función asíncrona para registrar el acceso
async def registrar_acceso(cedula, porteria, autenticado):
    # Fecha y hora actual
    fecha_acceso = datetime.now()
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
