from fastapi import FastAPI, Form

app = FastAPI()

@app.post("/api/fingerprint")
async def process_fingerprint(fingerprint: str = Form(...)):
    # Aquí se recibe la huella dactilar como un string hexadecimal
    print(f"Huella dactilar en formato hexadecimal: {fingerprint}")

    # Convertir el string hexadecimal de vuelta a bytes (si es necesario)
    fingerprint_data = bytes.fromhex(fingerprint)
    print(f"Tamaño de los datos recibidos: {len(fingerprint_data)} bytes")

    # Procesar la huella
    is_valid = True  # Aquí procesarías la huella dactilar

    return {"valid": is_valid}
