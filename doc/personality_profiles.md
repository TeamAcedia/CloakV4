````markdown
# Personality Profiles

This repository defines the **Personality Profile Format**, a simple INI-style specification for describing versioned profiles.

---

## 📑 Format

Each profile is defined inside a `[profile]` section with the following keys:

| Key                     | Type    | Required | Description |
|-------------------------|---------|----------|-------------|
| `profile_name`          | String  | ✅ Yes   | A descriptive name for the profile |
| `version_major`         | Integer | ✅ Yes   | Major version (breaking changes) |
| `version_minor`         | Integer | ✅ Yes   | Minor version (new features, backward compatible) |
| `version_patch`         | Integer | ✅ Yes   | Patch version (fixes, tweaks) |
| `version_extra`         | String  | ❌ No    | Extra tag (e.g. `dev`, `beta`, `alpha`, `rc1`) |
| `protocol_version_min`  | Integer | ✅ Yes   | Minimum compatible protocol version |
| `protocol_version_max`  | Integer | ✅ Yes   | Maximum compatible protocol version |

---

## 🧪 Example

```ini
[profile]
profile_name          = Luanti 5.13.0-dev
version_major         = 5
version_minor         = 13
version_patch         = 0
version_extra         = dev
protocol_version_min  = 37
protocol_version_max  = 48
````

---

## 📝 Notes

* The format is human-readable and easy to parse in most programming languages.
* Comment lines start with `#`.
* Duplicate keys are resolved by taking the **last definition**.

---