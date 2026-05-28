#!/usr/bin/env bash

set -euo pipefail

# -------------------------------
# Configuration
# -------------------------------

RUNS=($(seq 62179 62187))

DATA_DIR="/data1/yzhu/Projects/S2426/raw_data"
SORTER="./bin/s2426Sort"
OUT_DIR="./histOutput"

MAX_PARALLEL=1
RUN_LOG="${OUT_DIR}/run.log"

SAVE_FRAGMENT_LOG=0   # 1 = save printout to txt, 0 = print to terminal

mkdir -p "${OUT_DIR}"
: > "${RUN_LOG}"

# -------------------------------
# Function: process one run
# -------------------------------
process_run() {
  local run="$1"
  local files=()
  local rootfiles=()
  local frag_log=""

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

    frag_log="${OUT_DIR}/$(basename "${midfile}" .mid).txt"

    set +e
    if [ "${SAVE_FRAGMENT_LOG}" -eq 1 ]; then
      "${SORTER}" "${midfile}" > "${frag_log}" 2>&1
    else
      "${SORTER}" "${midfile}"
    fi
    status=$?
    set -e

    if [ ${status} -eq 0 ]; then
      echo "[INFO]   Finished $(basename "${midfile}")"

      if [ "${SAVE_FRAGMENT_LOG}" -eq 1 ]; then
        echo "[INFO]   Fragment log saved to ${frag_log}"
      fi
    else
      echo "[ERROR]   Failed on $(basename "${midfile}")"

      {
        echo "============================================================"
        echo "FILE: ${midfile}"
        echo "EXIT_STATUS: ${status}"
        echo "ERROR_MESSAGE:"

        if [ "${SAVE_FRAGMENT_LOG}" -eq 1 ]; then
          tail -n 30 "${frag_log}"
        else
          echo "Fragment log was not saved."
        fi

        echo ""
      } >> "${RUN_LOG}"

      continue
    fi
  done

  echo "[INFO] Run ${run} merging..."

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
export DATA_DIR SORTER OUT_DIR RUN_LOG SAVE_FRAGMENT_LOG

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
