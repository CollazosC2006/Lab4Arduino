from fastapi import FastAPI, HTTPException, Request
from pydantic import BaseModel
import json

app = FastAPI()

# Modelo para la estructura del JSON recibido
class AccessData(BaseModel):
    id: str
    access: str
    mac: str

@app.post("/verificar_acceso/")
async def verificar_acceso(data: AccessData):
    # Verificar el valor de "access" en el JSON
    print(data)
    if data.access == "1":
        return {"message": "Acceso concedido"}  # Respuesta 200 OK por defecto en FastAPI
    else:
        raise HTTPException(status_code=403, detail="Acceso denegado")  # Retorna 403 Forbidden
