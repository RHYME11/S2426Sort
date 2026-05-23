#!/usr/bin/env bash

set -euo pipefail

# -------------------------------
# Configuration
# -------------------------------

RUNS=(62183)

DATA_DIR="/data1/yzhu/Projects/S2426/raw_data"
SORTER="./bin/s2426Sort"
OUT_DIR="./histOutput"

MAX_PARALLEL=1
RUN_LOG="${OUT_DIR}/run.log"

mkdir -p "${OUT_DIR}"
: > "${RUN_LOG}"

# -------------------------------
# Function: process one run
# -------------------------------
process_run() {
  local run="$1"
  local files=()
  local rootfiles=()
  local tmp_log=""

  shopt -s nullglob
  files=(${DATA_DIR}/run${run}_*.mid)
  shopt -u nullglob

  if [ ${#files[@]} -eq 0 ]; then
    echo "[WARN] No subrun files for run ${run}"
    return 0
  fi

  echo "[INFO] Run ${run} start (${#files[@]} subruns)"

  for midfile in "${files[@]}"; do
    echo "[INFO]   Processing $(basename "${midfile}")"

    tmp_log="${OUT_DIR}/tmp_$(basename "${midfile}" .mid).log"

    set +e
    "${SORTER}" "${midfile}" > "${tmp_log}" 2>&1
    status=$?
    set -e

    if [ ${status} -eq 0 ]; then
      echo "[INFO]   Finished $(basename "${midfile}")"
      rm -f "${tmp_log}"
    else
      echo "[ERROR]   Failed on $(basename "${midfile}")"

      {
        echo "============================================================"
        echo "FILE: ${midfile}"
        echo "EXIT_STATUS: ${status}"
        echo "ERROR_MESSAGE:"
        tail -n 30 "${tmp_log}"
        echo ""
      } >> "${RUN_LOG}"

      rm -f "${tmp_log}"

      # Continue to next subrun instead of stopping this run
      continue
    fi
  done

  echo "[INFO] Run ${run} merging..."

  # Merge all ROOT files that were actually generated
  shopt -s nullglob
  rootfiles=(${OUT_DIR}/hist${run}_*.root)
  shopt -u nullglob

  if [ ${#rootfiles[@]} -gt 0 ]; then
    hadd -f "${OUT_DIR}/hist${run}.root" "${rootfiles[@]}"
    echo "[INFO] Run ${run} done"
  else
    echo "[WARN] No ROOT files generated for run ${run}, skip hadd"

    {
      echo "============================================================"
      echo "RUN: ${run}"
      echo "WARNING: No ROOT files generated, skip hadd"
      echo ""
    } >> "${RUN_LOG}"
  fi
}

export -f process_run
export DATA_DIR SORTER OUT_DIR RUN_LOG

# -------------------------------
# Parallel execution
# -------------------------------

printf "%s\n" "${RUNS[@]}" | \
xargs -I{} -P "${MAX_PARALLEL}" bash -c 'process_run "$@"' _ {}

# -------------------------------
# Cleanup: remove empty log file
# -------------------------------

if [ ! -s "${RUN_LOG}" ]; then
  rm -f "${RUN_LOG}"
  echo "[INFO] run_log is empty, removed"
else
  echo "[INFO] Failures/warnings saved in ${RUN_LOG}"
fi
