from fastapi import FastAPI, BackgroundTasks, HTTPException, Request
from pydantic import BaseModel
from pymongo import MongoClient
import secrets
from datetime import datetime
from bson import Int32
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
    id_registro = data.id
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
    # Identificar porteria con MAC
    porteria = collection_porterias.find_one({"mac":data.mac})
    if porteria:
        # Convertir el _id de la portería a int32
        porteria_id = Int32(porteria["_id"]) 
    else:
        # Si no se encuentra la mac asociada a la porteria
        raise HTTPException(status_code=403, detail="Punto de acceso (porteria) no encontrada")  # Retorna 403 Forbidden

    # Verificar el valor de "access" en el JSON
    print(data)
    if data.access == "1":
        # Registrar acceso exitoso en la base de datos
        background_tasks.add_task(registrar_acceso, data.id, porteria_id, True)
        # Imprimir información del usuario
        usuario_auth = collection_usuarios.find_one({"_id":data.id})
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
    # Generar 512 bytes aleatorios
    random_bytes = secrets.token_bytes(512)
    # Convertir a cadena hexadecimal
    hex_string = random_bytes.hex()
    return hex_string