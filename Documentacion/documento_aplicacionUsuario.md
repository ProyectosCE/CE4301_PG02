# Aplicación de Usuario y Comunicación

## Arquitectura de la Aplicación

La aplicación desarrollada para el proyecto GimmiCoin es una interfaz web embebida que permite a los usuarios interactuar con la alcancía inteligente. La arquitectura de la aplicación se basa en un servidor HTTP integrado en el microcontrolador ESP8266, que actúa como punto de acceso y servidor web.

### Componentes Principales

1. **Servidor Web**:
   - Implementado utilizando la librería `ESP8266WebServer`.
   - Proporciona endpoints para el control y monitoreo del sistema.

2. **Interfaz de Usuario**:
   - Diseñada en HTML y CSS embebidos en el firmware.
   - Compatible con dispositivos móviles y navegadores web.

3. **Persistencia**:
   - Uso de EEPROM para almacenar el contador de monedas de forma persistente.

## Interfaces de Usuario

La interfaz de usuario está diseñada para ser intuitiva y accesible desde cualquier dispositivo con capacidad Wi-Fi. Las principales vistas incluyen:

### Página Principal (`/`)

- **Contenido**:
  - Visualización del total de monedas registradas.
  - Estado del sensor de monedas.

- **Diseño**:
  - Estilo minimalista con colores suaves y tipografía legible.

```html
<!DOCTYPE html>
<html>
<head>
<title>Alcancía Inteligente</title>
<style>
  body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f5f9; }
  h1 { color: #0077cc; }
</style>
</head>
<body>
<h1>Total de monedas: 24</h1>
</body>
</html>
```

### Panel de Administración (`/admin`)

- **Contenido**:
  - Botón para reiniciar el contador de monedas.
  - Acceso protegido mediante autenticación básica (usuario: `admin`, contraseña: `1234`).

- **Funcionalidad**:
  - Permite a los administradores realizar un reinicio seguro del sistema.

## Protocolos de Comunicación Robot-Aplicación

### HTTP

- **Descripción**:
  - El protocolo HTTP se utiliza para la comunicación entre el cliente (navegador web) y el servidor embebido en el ESP8266.

- **Endpoints Implementados**:
  - `/` (GET): Devuelve la página principal con el estado actual.
  - `/estado` (GET): Proporciona un JSON con el estado del sistema.
  - `/admin` (GET): Página de administración protegida.
  - `/reset` (GET): Reinicia el contador de monedas.

### Ejemplo de Respuesta JSON (`/estado`)

```json
{
  "pulses": 24,
  "pressed": true
}
```

## Funcionalidades Implementadas

### Monitoreo

- **Descripción**:
  - Los usuarios pueden consultar el estado actual de la alcancía, incluyendo el número total de monedas registradas y el estado del sensor.

### Control

- **Descripción**:
  - Los administradores pueden reiniciar el contador de monedas a través del panel de administración.

### Seguridad

- **Autenticación**:
  - El acceso a las funciones administrativas está protegido mediante autenticación básica.

---

Esta aplicación web proporciona una solución sencilla y eficiente para el control y monitoreo de la alcancía inteligente, garantizando una experiencia de usuario fluida y segura.