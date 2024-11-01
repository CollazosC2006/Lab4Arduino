@echo off
cd C:\Users\colla\Desktop\Collazos\Universidad\Ing Elelctronica\Octavo Semestre\Lab 4 Electronica\Servidor
uvicorn main:app --workers 4 --host 0.0.0.0 --port 8000
pause
