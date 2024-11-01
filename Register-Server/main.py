from fastapi import FastAPI, Body
from pydantic import BaseModel  # Agregar esta línea


app = FastAPI()

# Modelo de datos esperado por el servidor
class TemplateData(BaseModel):
    cedula: str
    template1: str
    template2: str
    template3: str
    template4: str
    template5: str
    template6: str
    template7: str
    template8: str
    template9: str

class FingerprintRequest(BaseModel):
    templates: TemplateData

@app.post("/verificar_credenciales")
async def verificar_credenciales(data: FingerprintRequest):
    # Almacenar la cédula y las huellas en variables
    cedula = data.templates.cedula
    fingerprints = [
        data.templates.template1,
        data.templates.template2,
        data.templates.template3,
        data.templates.template4,
        data.templates.template5,
        data.templates.template6,
        data.templates.template7,
        data.templates.template8,
        data.templates.template9
    ]
    
    # Imprimir la cédula y las huellas dactilares
    print(f"Cédula: {cedula}")
    for idx, template in enumerate(fingerprints, start=1):
        print(f"Template {idx}: {template}")

    ######################COLOCAR REGISTRO CON ML AQUI##############################    
    
    return {"status": "200 OK"}
