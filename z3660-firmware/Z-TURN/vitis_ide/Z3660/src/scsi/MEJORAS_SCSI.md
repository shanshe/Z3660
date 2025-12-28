# Mejoras del módulo SCSI (Z3660/src/scsi)

Fecha: 2025-08-24
Ámbito: scsi.c, scsi.h, hunk-reloc.c, hunk-reloc.h, z3660_scsi_enums.h

Objetivo: Recopilar y priorizar mejoras de robustez, seguridad, rendimiento y mantenibilidad en el subsistema SCSI del proyecto Z3660.


Resumen de prioridades
- Alta: correcciones que previenen fallos, desbordamientos, fugas y corrupción de datos.
- Media: robustez, validaciones, consistencia de API, tratamiento de errores y límites.
- Baja: rendimiento fino, ordenamiento del código, mejoras de logging y observabilidad.


Prioridad alta (bugs y riesgos)
1) Índice de unidad fuera de rango
- Problema: en piscsi_map_drive(), el control usa if (index > NUM_SCSI_UNITS_MAX). Permite index == NUM_SCSI_UNITS_MAX, fuera de rango.
- Mejora: cambiar a if (index >= NUM_SCSI_UNITS_MAX) y devolver error claro.
- Ficheros: scsi.c

2) Valor inválido de piscsi_cur_drive (255) y accesos no validados
- Problema: PISCSI_CMD_DRVNUM puede asignar 255; varias lecturas usan devs[piscsi_cur_drive] sin validar.
- Mejora: no asignar valores inválidos; si la entrada es inválida, conservar el valor actual y devolver 0/ERR. Alternativamente, validar piscsi_cur_drive >= NUM_SCSI_UNITS_MAX en cada uso y abortar segura/rápidamente.
- Ficheros: scsi.c

3) Fugas de memoria en piscsi_find_filesystems()
- Problema: se hace malloc de fhb_block en el bucle y no siempre se libera (leak); sólo algunas copias se guardan en filesystems[].fhb.
- Mejora: usar un buffer de escaneo único (malloc una vez), y cuando se deba persistir un FSHD, copiarlo explícitamente a filesystems[i].fhb (malloc + memcpy). Liberar el buffer de escaneo al final.
- Ficheros: scsi.c

4) Tamaño y límites de relocalización (HUNK_RELOC)
- Problema: piscsi_hreloc es 2048 pero filesystems[i].relocs es 4096; hunk-reloc no limita el número de entradas y puede sobrescribir memoria si hay muchas relocs.
- Mejora: definir una constante global MAX_HUNK_RELOCS (por ejemplo, 4096) y:
  - Dimensionar piscsi_hreloc a MAX_HUNK_RELOCS.
  - En HUNKTYPE_HUNK_RELOC32, cortar si se supera MAX_HUNK_RELOCS y devolver error.
  - Propagar el error a quien llama.
- Ficheros: scsi.c, hunk-reloc.c, scsi.h (si se expone la constante)

5) Validación de block_size
- Problema: d->block_size proviene del RDB sin validar y se usa en malloc y lecturas.
- Mejora: validar 0 < block_size <= PISCSI_MAX_BLOCK_SIZE y que sea múltiplo de 512. Si no, decidir política: fijar 512 o abortar.
- Ficheros: scsi.c

6) Persistencia tras escrituras (f_sync)
- Problema: tras f_write no se llama a f_sync; riesgo de pérdida de datos en reset/cortes.
- Mejora: llamar a f_sync(d->fd) al finalizar operaciones de escritura de alto nivel o según umbral de bytes acumulados; opcionalmente controlable por configuración.
- Ficheros: scsi.c

7) Lecturas/escrituras RAW SD no alineadas a bloque
- Problema: en backend RAW (disk_read/disk_write) se calcula blocks = bytes / block_size y se ignora el resto.
- Mejora: definir política clara: o rechazar tamaños no alineados con error, o manejar el remanente con buffer temporal.
- Ficheros: scsi.c

8) Límite de particiones
- Problema: d->pb[16]; al seguir la lista enlazada PARTITIONLIST no se corta explícitamente tras 16.
- Mejora: si cur_partition alcanza 16, dejar de cargar y avisar.
- Ficheros: scsi.c


Robustez y corrección
1) Inclusión circular innecesaria
- Problema: hunk-reloc.h incluye scsi.h y scsi.h incluye hunk-reloc.h.
- Mejora: eliminar include de scsi.h en hunk-reloc.h (no es necesario); si se requieren tipos, reubicar typedefs o usar forward declarations.
- Ficheros: hunk-reloc.h

2) Firmas const-correct
- Mejora: io_cmd_name() y scsi_cmd_name() deben devolver const char*.
- Ficheros: scsi.c

3) Límite de filesystems
- Problema: piscsi_num_fs puede exceder NUM_FILESYSTEMS si hay demasiados FSHD.
- Mejora: comprobar y dejar de cargar (con log) al alcanzar NUM_FILESYSTEMS.
- Ficheros: scsi.c

4) Control de errores más consistente
- Observación: a veces se imprime error tras f_read/f_write/disk_read pero no se propaga y se continúa.
- Mejora: unificar estrategia: si un acceso a disco falla, propagar el error y abortar la operación (evitar relocalizaciones/copias con datos incompletos). Añadir códigos de retorno o enum de errores.
- Ficheros: scsi.c, hunk-reloc.c

5) Validación de punteros mágicos en backend
- Problema: se usa (FIL*)1 para distinguir RAW SD vs archivo.
- Mejora: añadir campo backend en PISCSI_DEV (FILE/HDF vs RAW_SD) y evitar comparaciones de punteros. Centralizar lógica de IO.
- Ficheros: scsi.h (enum/flag), scsi.c

6) Nombres de partición y límites de buffer
- Problema: al renombrar particiones duplicadas se puede exceder pb_DriveName[32].
- Mejora: usar snprintf y validar longitud; no exceder 31 chars + NUL. Actualizar pb->pb_DriveName[0] coherentemente.
- Ficheros: scsi.c

7) Validación de punteros/offsets de FSHD
- Mejora: validar que fhb->fhb_Next y rdb->rdb_FileSysHeaderList estén dentro del tamaño del dispositivo. Si se sale, abortar con aviso.
- Ficheros: scsi.c

8) Valores CHS fabricados
- Mejora: al inventar CHS en falta de RDB, evitar overflow y documentar el cálculo. Considerar 255 heads/63 secs si el tamaño es grande (compatibilidad común), o usar LBA puro en capas superiores.
- Ficheros: scsi.c


Rendimiento
1) Flushing/invalidating por rango
- Mejora: usar Xil_L1DCacheFlushRange/InvalidateRange con puntero + tamaño cuando se conoce el buffer, en lugar de flush global. Reduce latencias.
- Ficheros: scsi.c

2) Reutilización de buffers
- Mejora: reservar buffers reutilizables (p. ej., para bloques de tamaño d->block_size y para FSHD) y evitar malloc/free repetido en bucles.
- Ficheros: scsi.c

3) Tamaño de ráfagas de IO
- Mejora: MAX_BLOCKS_AT_ONCE a una constante configurable en cabecera. Ajustar a límites del controlador y tamaño de caché.
- Ficheros: scsi.c, scsi.h (si procede)

4) memcpy_neon firma y fallback
- Mejora: declarar extern void* memcpy_neon(void* restrict dst, const void* restrict src, uint32_t n); usar memcpy para tamaños pequeños o no alineados. Evitar u32 si no está definido.
- Ficheros: scsi.c (declaración);


Mantenibilidad
1) Helpers de mapeo de memoria
- Mejora: extraer helpers:
  - bool is_fpga_mapped_addr(uint32_t a, const CONFIG* cfg)
  - uint32_t translate_fpga_to_arm(uint32_t a)
  - bool in_cpu_ram_range(uint32_t a), bool in_autoconfig_ram_range(uint32_t a)
- Beneficio: eliminar duplicación y errores (p. ej. restar 0x20000000 para 0x40000000–0x4fffffff).
- Ficheros: scsi.c (nuevas funciones estáticas)

2) API de backend de IO unificada
- Mejora: crear piscsi_seek_bytes(), piscsi_read_bytes(), piscsi_write_bytes(), piscsi_read_blocks(), piscsi_write_blocks() que internen la diferencia FILE vs RAW SD.
- Beneficio: simplifica el manejo de errores, f_sync y métricas.
- Ficheros: scsi.c, scsi.h (si se expone)

3) Tipos y casts explícitos
- Mejora: usar size_t/FSIZE_t donde corresponda; añadir casts explícitos 32↔64 para offsets.
- Ficheros: scsi.c, hunk-reloc.c

4) Logging y rutas del usuario
- Mejora: centralizar DEBUG/printf bajo función que permita, opcionalmente, volcar a archivos de log del usuario:
  - /Volumes/datos/log.txt
  - /Volumes/datos/log_arm.txt
  - /Volumes/datos/log_trident.txt
- Añadir niveles (INFO/WARN/ERR/DEBUG) y toggles vía debug_console.
- Ficheros: scsi.c (envoltorio de logging)

5) Unificación de macros endian
- Mejora: unificar BE, BE16, etc., en una cabecera común y usar consistentemente.
- Ficheros: scsi.c, hunk-reloc.c, scsi.h


Calidad de parsing (RDB/FS)
1) Verificación de checksums
- Mejora: comprobar rdb_ChkSum/pb_ChkSum/fhb_ChkSum cuando sea viable para detectar corrupción.
- Ficheros: scsi.c

2) Límites de recorrido
- Mejora: límite superior en RDB_BLOCK_LIMIT configurable; detener si se encuentran valores improbables.
- Ficheros: scsi.h (const), scsi.c

3) Manejo de errores en LSEG
- Mejora: en load_lseg/process_hunks, propagar errores de I/O y truncar con mensajes claros.
- Ficheros: hunk-reloc.c


Quick wins (bajo riesgo, alto valor)
- [ ] Cambiar if (index > NUM_SCSI_UNITS_MAX) por if (index >= NUM_SCSI_UNITS_MAX) en piscsi_map_drive().
- [ ] Validar piscsi_cur_drive antes de acceder a devs[]. Si inválido, devolver 0/ERR y log.
- [ ] io_cmd_name()/scsi_cmd_name() → const char*.
- [ ] Eliminar include de scsi.h en hunk-reloc.h (evitar inclusión circular).
- [ ] Unificar MAX_HUNK_RELOCS a 4096 y dimensionar piscsi_hreloc con ese valor.
- [ ] Añadir guardas en HUNKTYPE_HUNK_RELOC32 para no exceder MAX_HUNK_RELOCS.
- [ ] Llamar a f_sync(d->fd) tras escrituras grandes o al finalizar comandos de escritura.
- [ ] Validar y normalizar block_size tras parsear RDB (múltiplo de 512 y ≤ PISCSI_MAX_BLOCK_SIZE); fallback a 512 o abortar.
- [ ] Límite de 16 particiones: cortar recorrido y avisar si se excede.
- [ ] Comprobar piscsi_num_fs contra NUM_FILESYSTEMS en piscsi_find_filesystems().
- [ ] Rechazar o manejar remanentes en RAW SD si bytes % block_size != 0.


Observabilidad (opcional pero útil)
- [ ] Contadores: bytes leídos/escritos, errores de I/O, tiempos por operación (medidos), número de flushes de caché.
- [ ] Traces de comandos: BeginIO, SCSICMD, READ/WRITE con tamaños y tiempos cuando el debug esté activo.


Plan sugerido de adopción por fases
Fase 1 (1–2 commits rápidos)
- Quick wins anteriores (validaciones, límites, const-correct, include, MAX_HUNK_RELOCS + guardas, block_size, f_sync). Riesgo bajo.

Fase 2 (robustez de I/O y parsing)
- Backend explícito en PISCSI_DEV y helpers de I/O.
- Reutilización de buffers + corrección de fugas en piscsi_find_filesystems().
- Validaciones extra de RDB/FS (offsets, checksums, límites de partición/FS).

Fase 3 (rendimiento y limpieza)
- Cache flush por rango, unificación de macros endian, logging configurable, métricas.


Notas de implementación y ubicación
- Validaciones de índice/cur_drive: scsi.c (handlers de lectura/escritura y piscsi_map_drive()).
- f_sync: scsi.c (ruta de WRITE/WRITE64/WRITEBYTES; un posible “end of command” natural tras completar la escritura).
- MAX_HUNK_RELOCS: definir en scsi.h o nueva cabecera; usar en scsi.c y hunk-reloc.c.
- Guardas en relocalización: hunk-reloc.c (HUNKTYPE_HUNK_RELOC32, cortar y retornar error si se excede el límite).
- Backend explícito: añadir enum/flag en PISCSI_DEV (scsi.h) y setearlo en piscsi_map_drive().
- Reutilización de buffers: scsi.c (buffers estáticos o miembros en PISCSI_DEV si conviene).
- Eliminación de include circular: hunk-reloc.h (quitar include de scsi.h).


Fragmento clave (selección del usuario)
- Control de errores más consistente
  • A veces se imprime error tras f_read/f_write/disk_read pero no se propaga el fallo ni se aborta la operación. Conviene unificar: si un acceso a disco falla, devolver error hacia arriba y no continuar con copias/relocs.


Checklist global
- [ ] Validar índices y estados antes de acceder a devs[] (incluyendo piscsi_cur_drive).
- [ ] Normalizar block_size y rechazar valores inválidos.
- [ ] Límite de particiones y filesystems respetados.
- [ ] Guardas de relocalización implementadas y tamaño de buffers consistente.
- [ ] Fugas de memoria eliminadas (buffers de escaneo FSHD, devs[i].pb[j]).
- [ ] Política de I/O RAW para tamaños no alineados decidida e implementada.
- [ ] f_sync aplicado donde corresponda.
- [ ] Cache flush por rango aplicado donde es posible.
- [ ] Backend explícito y helpers de I/O creados.
- [ ] Logging centralizado con niveles y posibilidad de volcar a rutas del usuario.


Notas finales
- Estas mejoras están pensadas para minimizar riesgos en datos y facilitar diagnóstico, manteniendo la compatibilidad.
- Las “quick wins” son seguras y recomendadas para aplicar primero.
- Las fases 2 y 3 pueden hacerse de forma incremental con pruebas entre cada cambio.

