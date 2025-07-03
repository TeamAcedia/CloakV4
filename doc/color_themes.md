# Color Theme Format

This document defines the structure and syntax for creating color themes using **HSL** or **HSLA** values.

---

## Format Structure

Each theme consists of a set of named color keys, defined in an INI-style `[theme]` section.

```ini
[theme]
name = Example Dark Theme

background-top    = hsl(220, 13%, 10%)
background        = hsl(220, 13%, 18%)
background-bottom = hsl(220, 13%, 25%)

border            = hsl(220, 13%, 40%)

text              = hsl(0, 0%, 100%)
text-muted        = hsl(0, 0%, 70%)

primary           = hsl(210, 90%, 60%)
primary-muted     = hsla(210, 90%, 35%, 0.8)

secondary         = hsl(0, 90%, 60%)
secondary-muted   = hsla(0, 90%, 35%, 0.8)
```

---

## Required Keys

Each theme must define the following keys:

* `background-top`
* `background`
* `background-bottom`
* `border`
* `text`
* `text-muted`
* `primary`
* `primary-muted`

---

## Color Value Formats

* Use **HSL** or **HSLA** formats:

  * `hsl(H, S%, L%)`   → No transparency
  * `hsla(H, S%, L%, A)` → With alpha (opacity)

### 🧪 Examples

```ini
text            = hsl(0, 0%, 100%)      ; white
primary         = hsl(210, 90%, 60%)    ; strong blue
primary-muted   = hsla(210, 90%, 60%, 0.5) ; 50% opacity
```

---

## Notes

* Percent signs are **required** for saturation and lightness.
* Alpha (`A`) in `hsla` must be a float between `0.0` and `1.0`.
* Duplicate keys will be **overwritten** by the last occurrence.
* Lines starting with `#` are treated as comments.

---