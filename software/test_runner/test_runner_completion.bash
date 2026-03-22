# Bash completion for test_runner.py

# Add 
# source "$HOME/tf/esp32-firmware/software/test_runner/test_runner_completion.bash"
# to your .bashrc for test_runner tab completion

_test_runner_completions() {
    local cur="${COMP_WORDS[COMP_CWORD]}"

    # For flags, use default completion
    if [[ "$cur" == -* ]]; then
        COMPREPLY=($(compgen -W "--junit-xml --host --tty --brickd" -- "$cur"))
        return
    fi

    # Skip if previous word expects a value
    local prev="${COMP_WORDS[COMP_CWORD-1]}"
    if [[ "$prev" == "--host" || "$prev" == "--tty" || "$prev" == "--brickd" ]]; then
        return
    fi

    # Find the test_runner directory relative to the script being completed
    local runner_dir
    for word in "${COMP_WORDS[@]}"; do
        if [[ "$word" == *test_runner.py ]]; then
            runner_dir="$(dirname "$word")"
            break
        fi
    done
    if [[ -z "$runner_dir" ]]; then
        return
    fi

    local modules_dir="$runner_dir/../src/modules"

    # Count slashes to determine completion level
    local slashes="${cur//[^\/]/}"
    local n_slashes="${#slashes}"

    case "$n_slashes" in
        0)
            # Complete module names (only modules that have a tests/ dir)
            local modules=()
            for d in "$modules_dir"/*/tests/; do
                if [[ -d "$d" ]]; then
                    local mod="$(basename "$(dirname "$d")")"
                    modules+=("$mod/")
                fi
            done
            # Also offer test_runner/ if it has tests
            if [[ -d "$runner_dir/tests" ]]; then
                modules+=("test_runner/")
            fi
            compopt -o nospace
            COMPREPLY=($(compgen -W "${modules[*]}" -- "$cur"))
            ;;
        1)
            # Complete suite names within a module
            local module="${cur%%/*}"
            local suite_prefix="${cur#*/}"
            local suites=()

            local test_dir="$modules_dir/$module/tests"
            if [[ "$module" == "test_runner" ]]; then
                test_dir="$runner_dir/tests"
            fi

            if [[ -d "$test_dir" ]]; then
                for f in "$test_dir"/*.py; do
                    local base="$(basename "$f" .py)"
                    # Skip _-prefixed helpers and check for run_testsuite
                    [[ "$base" == _* ]] && continue
                    grep -q 'run_testsuite' "$f" 2>/dev/null || continue
                    suites+=("$module/$base/")
                done
            fi
            compopt -o nospace
            COMPREPLY=($(compgen -W "${suites[*]}" -- "$cur"))
            ;;
        2)
            # Complete test names within a suite
            local module="${cur%%/*}"
            local rest="${cur#*/}"
            local suite="${rest%%/*}"
            local test_prefix="${rest#*/}"

            local test_dir="$modules_dir/$module/tests"
            if [[ "$module" == "test_runner" ]]; then
                test_dir="$runner_dir/tests"
            fi

            local suite_file="$test_dir/$suite.py"
            local tests=()
            if [[ -f "$suite_file" ]]; then
                while IFS= read -r line; do
                    if [[ "$line" =~ ^def\ (test_[a-zA-Z0-9_]*) ]]; then
                        tests+=("$module/$suite/${BASH_REMATCH[1]}")
                    fi
                done < "$suite_file"
                # Also offer the wildcard
                tests+=("$module/$suite/*")
            fi
            COMPREPLY=($(compgen -W "${tests[*]}" -- "$cur"))
            ;;
    esac
}

complete -F _test_runner_completions test_runner.py
complete -F _test_runner_completions ./test_runner.py
complete -F _test_runner_completions ./test_runner/test_runner.py
