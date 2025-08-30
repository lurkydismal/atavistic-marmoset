#pragma once

#include <bits/exception_ptr.h>

#include <string>

namespace Assimp {

struct aiTexture {
    /** Width of the texture, in pixels
     *
     * If mHeight is zero the texture is compressed in a format
     * like JPEG. In this case mWidth specifies the size of the
     * memory area pcData is pointing to, in bytes.
     */
    unsigned int mWidth;

    /** Height of the texture, in pixels
     *
     * If this value is zero, pcData points to an compressed texture
     * in any format (e.g. JPEG).
     */
    unsigned int mHeight;

    /** A hint from the loader to make it easier for applications
     *  to determine the type of embedded textures.
     *
     * If mHeight != 0 this member is show how data is packed. Hint will consist
     * of two parts: channel order and channel bitness (count of the bits for
     * every color channel). For simple parsing by the viewer it's better to not
     * omit absent color channel and just use 0 for bitness. For example:
     * 1. Image contain RGBA and 8 bit per channel, achFormatHint == "rgba8888";
     * 2. Image contain ARGB and 8 bit per channel, achFormatHint == "argb8888";
     * 3. Image contain RGB and 5 bit for R and B channels and 6 bit for G
     * channel, achFormatHint == "rgba5650";
     * 4. One color image with B channel and 1 bit for it, achFormatHint ==
     * "rgba0010"; If mHeight == 0 then achFormatHint is set set to
     * '\\0\\0\\0\\0' if the loader has no additional information about the
     * texture file format used OR the file extension of the format without a
     * trailing dot. If there are multiple file extensions for a format, the
     * shortest extension is chosen (JPEG maps to 'jpg', not to 'jpeg'). E.g.
     * 'dds\\0', 'pcx\\0', 'jpg\\0'.  All characters are lower-case. The fourth
     * character will always be '\\0'.
     */
    char achFormatHint[ HINTMAXTEXTURELEN ]; // 8 for string + 1 for terminator.

    /** Data of the texture.
     *
     * Points to an array of mWidth * mHeight aiTexel's.
     * The format of the texture data shall always be ARGB8888 if the
     * texture-hint of the type is empty. If the hint is not empty you can
     * interpret the format by looking into this hint. make the implementation
     * for user of the library as easy as possible. If mHeight = 0 this is a
     * pointer to a memory buffer of size mWidth containing the compressed
     * texture data. Good luck, have fun!
     */
    aiTexel* pcData;

    /** Texture original filename
     *
     * Used to get the texture reference
     */
    aiString mFilename;

    //! For compressed textures (mHeight == 0): compare the
    //! format hint against a given string.
    //! @param s Input string. 3 characters are maximally processed.
    //!        Example values: "jpg", "png"
    //! @return true if the given string matches the format hint
    bool CheckFormat( const char* s ) const {
        if ( nullptr == s ) {
            return false;
        }

        return ( 0 == ::strncmp( achFormatHint, s, sizeof( achFormatHint ) ) );
    }

    // Construction
    aiTexture() AI_NO_EXCEPT : mWidth( 0 ),
                               mHeight( 0 ),
                               pcData( nullptr ),
                               mFilename() {
        memset( achFormatHint, 0, sizeof( achFormatHint ) );
    }

    // Destruction
    ~aiTexture() { delete[] pcData; }
};

using ai_real = float;
using ai_int = signed int;
using ai_uint = unsigned int;

constexpr ai_real ai_epsilon = ( ai_real )1e-6;

inline constexpr const float AI_CONFIG_CHECK_IDENTITY_MATRIX_EPSILON_DEFAULT =
    10e-3f;

template < typename TReal >
class aiVector3t;

using aiVector3D = aiVector3t< ai_real >;
using aiVector3f = aiVector3t< float >;
using aiVector3d = aiVector3t< double >;

template < typename TReal >
class aiMatrix3x3t;
template < typename TReal >
class aiQuaterniont;

template < typename TReal >
class aiMatrix4x4t {
public:
    /**
     * @brief Set to identity
     * */
    aiMatrix4x4t() noexcept;

    /**
     * @brief Construction from single values
     * */
    aiMatrix4x4t( TReal _a1,
                  TReal _a2,
                  TReal _a3,
                  TReal _a4,
                  TReal _b1,
                  TReal _b2,
                  TReal _b3,
                  TReal _b4,
                  TReal _c1,
                  TReal _c2,
                  TReal _c3,
                  TReal _c4,
                  TReal _d1,
                  TReal _d2,
                  TReal _d3,
                  TReal _d4 );

    /** construction from 3x3 matrix, remaining elements are set to identity */
    explicit aiMatrix4x4t( const aiMatrix3x3t< TReal >& m );

    /** construction from position, rotation and scaling components
     * @param scaling The scaling for the x,y,z axes
     * @param rotation The rotation as a hamilton quaternion
     * @param position The position for the x,y,z axes
     */
    aiMatrix4x4t( const aiVector3t< TReal >& scaling,
                  const aiQuaterniont< TReal >& rotation,
                  const aiVector3t< TReal >& position );

    // array access operators
    /** @fn TReal* operator[] (unsigned int p_iIndex)
     *  @param [in] p_iIndex - index of the row.
     *  @return pointer to pointed row.
     */
    TReal* operator[]( unsigned int p_iIndex );

    /** @fn const TReal* operator[] (unsigned int p_iIndex) const
     *  @overload TReal* operator[] (unsigned int p_iIndex)
     */
    const TReal* operator[]( unsigned int p_iIndex ) const;

    // comparison operators
    bool operator==( const aiMatrix4x4t& m ) const;
    bool operator!=( const aiMatrix4x4t& m ) const;

    bool Equal( const aiMatrix4x4t& m, TReal epsilon = ai_epsilon ) const;

    // matrix multiplication.
    aiMatrix4x4t& operator*=( const aiMatrix4x4t& m );
    aiMatrix4x4t operator*( const aiMatrix4x4t& m ) const;
    aiMatrix4x4t operator*( const TReal& aFloat ) const;
    aiMatrix4x4t operator+( const aiMatrix4x4t& aMatrix ) const;

    template < typename TOther >
    operator aiMatrix4x4t< TOther >() const;

    // -------------------------------------------------------------------
    /** @brief Transpose the matrix */
    aiMatrix4x4t& Transpose();

    // -------------------------------------------------------------------
    /** @brief Invert the matrix.
     *  If the matrix is not invertible all elements are set to qnan.
     *  Beware, use (f != f) to check whether a TReal f is qnan.
     */
    aiMatrix4x4t& Inverse();

    // -------------------------------------------------------------------
    /**
     * @brief Inverts the matrix if it is invertible.
     */
    TReal Determinant() const;

    // -------------------------------------------------------------------
    /** @brief Returns true of the matrix is the identity matrix.
     *  @param epsilon Value of epsilon. Default value is 10e-3 for backward
     *  compatibility with legacy code.
     *  @return Returns true of the matrix is the identity matrix.
     *  The check is performed against a not so small epsilon.
     */
    inline bool IsIdentity(
        const TReal epsilon =
            AI_CONFIG_CHECK_IDENTITY_MATRIX_EPSILON_DEFAULT ) const;

    // -------------------------------------------------------------------
    /** @brief Decompose a trafo matrix into its original components
     *  @param scaling  Receives the output scaling for the x,y,z axes
     *  @param rotation Receives the output rotation as a hamilton quaternion
     *  @param position Receives the output position for the x,y,z axes
     */
    void Decompose( aiVector3t< TReal >& scaling,
                    aiQuaterniont< TReal >& rotation,
                    aiVector3t< TReal >& position ) const;

    // -------------------------------------------------------------------
    /**
     *  @brief Decompose a trafo matrix into its original components.
     *  Thx to good FAQ at http://www.gamedev.ru/code/articles/faq_matrix_quat
     *  @param [out] pScaling  - Receives the output scaling for the x,y,z axes.
     *  @param [out] pRotation - Receives the output rotation as a Euler angles.
     *  @param [out] pPosition - Receives the output position for the x,y,z
     * axes.
     */
    void Decompose( aiVector3t< TReal >& pScaling,
                    aiVector3t< TReal >& pRotation,
                    aiVector3t< TReal >& pPosition ) const;

    // -------------------------------------------------------------------
    /**
     *  @brief Decompose a trafo matrix into its original components
     * Thx to good FAQ at http://www.gamedev.ru/code/articles/faq_matrix_quat
     *  @param [out] pScaling       - Receives the output scaling for the x,y,z
     * axes.
     *  @param [out] pRotationAxis  - Receives the output rotation axis.
     *  @param [out] pRotationAngle - Receives the output rotation angle for
     * @ref pRotationAxis.
     *  @param [out] pPosition      - Receives the output position for the x,y,z
     * axes.
     */
    void Decompose( aiVector3t< TReal >& pScaling,
                    aiVector3t< TReal >& pRotationAxis,
                    TReal& pRotationAngle,
                    aiVector3t< TReal >& pPosition ) const;

    // -------------------------------------------------------------------
    /** @brief Decompose a trafo matrix with no scaling into its
     *    original components
     *  @param rotation Receives the output rotation as a hamilton quaternion
     *  @param position Receives the output position for the x,y,z axes
     */
    void DecomposeNoScaling( aiQuaterniont< TReal >& rotation,
                             aiVector3t< TReal >& position ) const;

    // -------------------------------------------------------------------
    /** @brief Creates a trafo matrix from a set of euler angles
     *  @param x Rotation angle for the x-axis, in radians
     *  @param y Rotation angle for the y-axis, in radians
     *  @param z Rotation angle for the z-axis, in radians
     */
    aiMatrix4x4t& FromEulerAnglesXYZ( TReal x, TReal y, TReal z );
    aiMatrix4x4t& FromEulerAnglesXYZ( const aiVector3t< TReal >& blubb );

    // -------------------------------------------------------------------
    /** @brief Returns a rotation matrix for a rotation around the x axis
     *  @param a   Rotation angle, in radians
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& RotationX( TReal a, aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** @brief Returns a rotation matrix for a rotation around the y axis
     *  @param a   Rotation angle, in radians
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& RotationY( TReal a, aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** @brief Returns a rotation matrix for a rotation around the z axis
     *  @param a Rotation angle, in radians
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& RotationZ( TReal a, aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** Returns a rotation matrix for a rotation around an arbitrary axis.
     *  @param a Rotation angle, in radians
     *  @param axis Rotation axis, should be a normalized vector.
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& Rotation( TReal a,
                                   const aiVector3t< TReal >& axis,
                                   aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** @brief Returns a translation matrix
     *  @param v Translation vector
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& Translation( const aiVector3t< TReal >& v,
                                      aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** @brief Returns a scaling matrix
     *  @param v Scaling vector
     *  @param out Receives the output matrix
     *  @return Reference to the output matrix
     */
    static aiMatrix4x4t& Scaling( const aiVector3t< TReal >& v,
                                  aiMatrix4x4t& out );

    // -------------------------------------------------------------------
    /** @brief A function for creating a rotation matrix that rotates a
     *  vector called "from" into another vector called "to".
     * Input : from[3], to[3] which both must be *normalized* non-zero vectors
     * Output: mtx[3][3] -- a 3x3 matrix in column-major form
     * Authors: Tomas Mueller, John Hughes
     *          "Efficiently Building a Matrix to Rotate One Vector to Another"
     *          Journal of Graphics Tools, 4(4):1-4, 1999
     */
    static aiMatrix4x4t& FromToMatrix( const aiVector3t< TReal >& from,
                                       const aiVector3t< TReal >& to,
                                       aiMatrix4x4t& out );

    TReal a1, a2, a3, a4;
    TReal b1, b2, b3, b4;
    TReal c1, c2, c3, c4;
    TReal d1, d2, d3, d4;
};

using aiMatrix4x4 = aiMatrix4x4t< ai_real >;

using aiReturn = enum aiReturn {
    aiReturn_SUCCESS = 0x0,
    aiReturn_FAILURE = -0x1,
    aiReturn_OUTOFMEMORY = -0x3,

    //  Force 32-bit size enum
    _AI_ENFORCE_ENUM_SIZE = 0x7fffffff
};

struct aiScene;
class BaseImporter;
class BaseProcess;
class SharedPostProcessInfo;
class BatchLoader;
class IOSystem;
class ProgressHandler;
struct aiString;
struct aiImporterDesc;
struct aiMemoryInfo;
class ImporterPimpl;

class Importer {
public:
    /**
     *  @brief The upper limit for hints.
     */
    static const unsigned int MaxLenHint = 200;

public:
    // -------------------------------------------------------------------
    /** Constructor. Creates an empty importer object.
     *
     * Call ReadFile() to start the import process. The configuration
     * property table is initially empty.
     */
    Importer();

    // -------------------------------------------------------------------
    /** Copy constructor.
     *
     * This copies the configuration properties of another Importer.
     * If this Importer owns a scene it won't be copied.
     * Call ReadFile() to start the import process.
     */
    Importer( const Importer& other ) = delete;

    // -------------------------------------------------------------------
    /** Assignment operator has been deleted
     */
    Importer& operator=( const Importer& ) = delete;

    // -------------------------------------------------------------------
    /** Destructor. The object kept ownership of the imported data,
     * which now will be destroyed along with the object.
     */
    ~Importer();

    // -------------------------------------------------------------------
    /** Registers a new loader.
     *
     * @param pImp Importer to be added. The Importer instance takes
     *   ownership of the pointer, so it will be automatically deleted
     *   with the Importer instance.
     * @return AI_SUCCESS if the loader has been added. The registration
     *   fails if there is already a loader for a specific file extension.
     */
    aiReturn RegisterLoader( BaseImporter* pImp );

    // -------------------------------------------------------------------
    /** Unregisters a loader.
     *
     * @param pImp Importer to be unregistered.
     * @return AI_SUCCESS if the loader has been removed. The function
     *   fails if the loader is currently in use (this could happen
     *   if the #Importer instance is used by more than one thread) or
     *   if it has not yet been registered.
     */
    aiReturn UnregisterLoader( BaseImporter* pImp );

    // -------------------------------------------------------------------
    /** Registers a new post-process step.
     *
     * At the moment, there's a small limitation: new post processing
     * steps are added to end of the list, or in other words, executed
     * last, after all built-in steps.
     * @param pImp Post-process step to be added. The Importer instance
     *   takes ownership of the pointer, so it will be automatically
     *   deleted with the Importer instance.
     * @return AI_SUCCESS if the step has been added correctly.
     */
    aiReturn RegisterPPStep( BaseProcess* pImp );

    // -------------------------------------------------------------------
    /** Unregisters a post-process step.
     *
     * @param pImp Step to be unregistered.
     * @return AI_SUCCESS if the step has been removed. The function
     *   fails if the step is currently in use (this could happen
     *   if the #Importer instance is used by more than one thread) or
     *   if it has not yet been registered.
     */
    aiReturn UnregisterPPStep( BaseProcess* pImp );

    // -------------------------------------------------------------------
    /** Set an integer configuration property.
     * @param szName Name of the property. All supported properties
     *   are defined in the aiConfig.g header (all constants share the
     *   prefix AI_CONFIG_XXX and are simple strings).
     * @param iValue New value of the property
     * @return true if the property was set before. The new value replaces
     *   the previous value in this case.
     * @note Property of different types (float, int, string ..) are kept
     *   on different stacks, so calling SetPropertyInteger() for a
     *   floating-point property has no effect - the loader will call
     *   GetPropertyFloat() to read the property, but it won't be there.
     */
    bool SetPropertyInteger( const char* szName, int iValue );

    // -------------------------------------------------------------------
    /** Set a boolean configuration property. Boolean properties
     *  are stored on the integer stack internally so it's possible
     *  to set them via #SetPropertyBool and query them with
     *  #GetPropertyBool and vice versa.
     * @see SetPropertyInteger()
     */
    bool SetPropertyBool( const char* szName, bool value ) {
        return SetPropertyInteger( szName, value );
    }

    // -------------------------------------------------------------------
    /** Set a floating-point configuration property.
     * @see SetPropertyInteger()
     */
    bool SetPropertyFloat( const char* szName, ai_real fValue );

    // -------------------------------------------------------------------
    /** Set a string configuration property.
     * @see SetPropertyInteger()
     */
    bool SetPropertyString( const char* szName, const std::string& sValue );

    // -------------------------------------------------------------------
    /** Set a matrix configuration property.
     * @see SetPropertyInteger()
     */
    bool SetPropertyMatrix( const char* szName, const aiMatrix4x4& sValue );

    // -------------------------------------------------------------------
    /** Set a pointer configuration property.
     * @see SetPropertyInteger()
     */
    bool SetPropertyPointer( const char* szName, void* sValue );

    // -------------------------------------------------------------------
    /** Get a configuration property.
     * @param szName Name of the property. All supported properties
     *   are defined in the aiConfig.g header (all constants share the
     *   prefix AI_CONFIG_XXX).
     * @param iErrorReturn Value that is returned if the property
     *   is not found.
     * @return Current value of the property
     * @note Property of different types (float, int, string ..) are kept
     *   on different lists, so calling SetPropertyInteger() for a
     *   floating-point property has no effect - the loader will call
     *   GetPropertyFloat() to read the property, but it won't be there.
     */
    int GetPropertyInteger( const char* szName,
                            int iErrorReturn = 0xffffffff ) const;

    // -------------------------------------------------------------------
    /** Get a boolean configuration property. Boolean properties
     *  are stored on the integer stack internally so it's possible
     *  to set them via #SetPropertyBool and query them with
     *  #GetPropertyBool and vice versa.
     * @see GetPropertyInteger()
     */
    bool GetPropertyBool( const char* szName,
                          bool bErrorReturn = false ) const {
        return GetPropertyInteger( szName, bErrorReturn ) != 0;
    }

    // -------------------------------------------------------------------
    /** Get a floating-point configuration property
     * @see GetPropertyInteger()
     */
    ai_real GetPropertyFloat( const char* szName,
                              ai_real fErrorReturn = 10e10 ) const;

    // -------------------------------------------------------------------
    /** Get a string configuration property
     *
     *  The return value remains valid until the property is modified.
     * @see GetPropertyInteger()
     */
    std::string GetPropertyString(
        const char* szName,
        const std::string& sErrorReturn = std::string() ) const;

    // -------------------------------------------------------------------
    /** Get a matrix configuration property
     *
     *  The return value remains valid until the property is modified.
     * @see GetPropertyInteger()
     */
    aiMatrix4x4 GetPropertyMatrix(
        const char* szName,
        const aiMatrix4x4& sErrorReturn = aiMatrix4x4() ) const;

    // -------------------------------------------------------------------
    /** Get a pointer configuration property
     *
     *  The return value remains valid until the property is modified.
     * @see GetPropertyInteger()
     */
    void* GetPropertyPointer( const char* szName,
                              void* sErrorReturn = nullptr ) const;

    // -------------------------------------------------------------------
    /** Supplies a custom IO handler to the importer to use to open and
     * access files. If you need the importer to use custom IO logic to
     * access the files, you need to provide a custom implementation of
     * IOSystem and IOFile to the importer. Then create an instance of
     * your custom IOSystem implementation and supply it by this function.
     *
     * The Importer takes ownership of the object and will destroy it
     * afterwards. The previously assigned handler will be deleted.
     * Pass nullptr to take again ownership of your IOSystem and reset Assimp
     * to use its default implementation.
     *
     * @param pIOHandler The IO handler to be used in all file accesses
     *   of the Importer.
     */
    void SetIOHandler( IOSystem* pIOHandler );

    // -------------------------------------------------------------------
    /** Retrieves the IO handler that is currently set.
     * You can use #IsDefaultIOHandler() to check whether the returned
     * interface is the default IO handler provided by ASSIMP. The default
     * handler is active as long the application doesn't supply its own
     * custom IO handler via #SetIOHandler().
     * @return A valid IOSystem interface, never nullptr.
     */
    IOSystem* GetIOHandler() const;

    // -------------------------------------------------------------------
    /** Checks whether a default IO handler is active
     * A default handler is active as long the application doesn't
     * supply its own custom IO handler via #SetIOHandler().
     * @return true by default
     */
    bool IsDefaultIOHandler() const;

    // -------------------------------------------------------------------
    /** Supplies a custom progress handler to the importer. This
     *  interface exposes an #Update() callback, which is called
     *  more or less periodically (please don't sue us if it
     *  isn't as periodically as you'd like it to have ...).
     *  This can be used to implement progress bars and loading
     *  timeouts.
     *  @param pHandler Progress callback interface. Pass nullptr to
     *    disable progress reporting.
     *  @note Progress handlers can be used to abort the loading
     *    at almost any time.*/
    void SetProgressHandler( ProgressHandler* pHandler );

    // -------------------------------------------------------------------
    /** Retrieves the progress handler that is currently set.
     * You can use #IsDefaultProgressHandler() to check whether the returned
     * interface is the default handler provided by ASSIMP. The default
     * handler is active as long the application doesn't supply its own
     * custom handler via #SetProgressHandler().
     * @return A valid ProgressHandler interface, never nullptr.
     */
    ProgressHandler* GetProgressHandler() const;

    // -------------------------------------------------------------------
    /** Checks whether a default progress handler is active
     * A default handler is active as long the application doesn't
     * supply its own custom progress handler via #SetProgressHandler().
     * @return true by default
     */
    bool IsDefaultProgressHandler() const;

    // -------------------------------------------------------------------
    /** @brief Check whether a given set of post-processing flags
     *  is supported.
     *
     *  Some flags are mutually exclusive, others are probably
     *  not available because your excluded them from your
     *  Assimp builds. Calling this function is recommended if
     *  you're unsure.
     *
     *  @param pFlags Bitwise combination of the aiPostProcess flags.
     *  @return true if this flag combination is fine.
     */
    bool ValidateFlags( unsigned int pFlags ) const;

    // -------------------------------------------------------------------
    /** Reads the given file and returns its contents if successful.
     *
     * If the call succeeds, the contents of the file are returned as a
     * pointer to an aiScene object. The returned data is intended to be
     * read-only, the importer object keeps ownership of the data and will
     * destroy it upon destruction. If the import fails, nullptr is returned.
     * A human-readable error description can be retrieved by calling
     * GetErrorString(). The previous scene will be deleted during this call.
     * @param pFile Path and filename to the file to be imported.
     * @param pFlags Optional post processing steps to be executed after
     *   a successful import. Provide a bitwise combination of the
     *   #aiPostProcessSteps flags. If you wish to inspect the imported
     *   scene first in order to fine-tune your post-processing setup,
     *   consider to use #ApplyPostProcessing().
     * @return A pointer to the imported data, nullptr if the import failed.
     *   The pointer to the scene remains in possession of the Importer
     *   instance. Use GetOrphanedScene() to take ownership of it.
     *
     * @note Assimp is able to determine the file format of a file
     * automatically.
     */
    const aiScene* ReadFile( const char* pFile, unsigned int pFlags );

    // -------------------------------------------------------------------
    /** Reads the given file from a memory buffer and returns its
     *  contents if successful.
     *
     * If the call succeeds, the contents of the file are returned as a
     * pointer to an aiScene object. The returned data is intended to be
     * read-only, the importer object keeps ownership of the data and will
     * destroy it upon destruction. If the import fails, nullptr is returned.
     * A human-readable error description can be retrieved by calling
     * GetErrorString(). The previous scene will be deleted during this call.
     * Calling this method doesn't affect the active IOSystem.
     * @param pBuffer Pointer to the file data
     * @param pLength Length of pBuffer, in bytes
     * @param pFlags Optional post processing steps to be executed after
     *   a successful import. Provide a bitwise combination of the
     *   #aiPostProcessSteps flags. If you wish to inspect the imported
     *   scene first in order to fine-tune your post-processing setup,
     *   consider to use #ApplyPostProcessing().
     * @param pHint An additional hint to the library. If this is a non
     *   empty string, the library looks for a loader to support
     *   the file extension specified by pHint and passes the file to
     *   the first matching loader. If this loader is unable to completely
     *   the request, the library continues and tries to determine the
     *   file format on its own, a task that may or may not be successful.
     *   Check the return value, and you'll know ...
     * @return A pointer to the imported data, nullptr if the import failed.
     *   The pointer to the scene remains in possession of the Importer
     *   instance. Use GetOrphanedScene() to take ownership of it.
     *
     * @note This is a straightforward way to decode models from memory
     * buffers, but it doesn't handle model formats that spread their
     * data across multiple files or even directories. Examples include
     * OBJ or MD3, which outsource parts of their material info into
     * external scripts. If you need full functionality, provide
     * a custom IOSystem to make Assimp find these files and use
     * the regular ReadFile() API.
     */
    const aiScene* ReadFileFromMemory( const void* pBuffer,
                                       size_t pLength,
                                       unsigned int pFlags,
                                       const char* pHint = "" );

    // -------------------------------------------------------------------
    /** Apply post-processing to an already-imported scene.
     *
     *  This is strictly equivalent to calling #ReadFile() with the same
     *  flags. However, you can use this separate function to inspect
     *  the imported scene first to fine-tune your post-processing setup.
     *  @param pFlags Provide a bitwise combination of the
     *   #aiPostProcessSteps flags.
     *  @return A pointer to the post-processed data. This is still the
     *   same as the pointer returned by #ReadFile(). However, if
     *   post-processing fails, the scene could now be nullptr.
     *   That's quite a rare case, post processing steps are not really
     *   designed to 'fail'. To be exact, the #aiProcess_ValidateDS
     *   flag is currently the only post processing step which can actually
     *   cause the scene to be reset to nullptr.
     *
     *  @note The method does nothing if no scene is currently bound
     *    to the #Importer instance.  */
    const aiScene* ApplyPostProcessing( unsigned int pFlags );

    const aiScene* ApplyCustomizedPostProcessing( BaseProcess* rootProcess,
                                                  bool requestValidation );

    // -------------------------------------------------------------------
    /** @brief Reads the given file and returns its contents if successful.
     *
     * This function is provided for backward compatibility.
     * See the const char* version for detailed docs.
     * @see ReadFile(const char*, pFlags)  */
    const aiScene* ReadFile( const std::string& pFile, unsigned int pFlags );

    // -------------------------------------------------------------------
    /** Frees the current scene.
     *
     *  The function does nothing if no scene has previously been
     *  read via ReadFile(). FreeScene() is called automatically by the
     *  destructor and ReadFile() itself.  */
    void FreeScene();

    // -------------------------------------------------------------------
    /** Returns an error description of an error that occurred in ReadFile().
     *
     * Returns an empty string if no error occurred.
     * @return A description of the last error, an empty string if no
     *   error occurred. The string is never nullptr.
     *
     * @note The returned function remains valid until one of the
     * following methods is called: #ReadFile(), #FreeScene(). */
    const char* GetErrorString() const;

    // -------------------------------------------------------------------
    /** Returns an exception if one occurred during import.
     *
     * @return The last exception which occurred.
     *
     * @note The returned value remains valid until one of the
     * following methods is called: #ReadFile(), #FreeScene(). */
    const std::exception_ptr& GetException() const;

    // -------------------------------------------------------------------
    /** Returns the scene loaded by the last successful call to ReadFile()
     *
     * @return Current scene or nullptr if there is currently no scene loaded */
    const aiScene* GetScene() const;

    // -------------------------------------------------------------------
    /** Returns the scene loaded by the last successful call to ReadFile()
     *  and releases the scene from the ownership of the Importer
     *  instance. The application is now responsible for deleting the
     *  scene. Any further calls to GetScene() or GetOrphanedScene()
     *  will return nullptr - until a new scene has been loaded via ReadFile().
     *
     * @return Current scene or nullptr if there is currently no scene loaded
     * @note Use this method with maximal caution, and only if you have to.
     *   By design, aiScene's are exclusively maintained, allocated and
     *   deallocated by Assimp and no one else. The reasoning behind this
     *   is the golden rule that deallocations should always be done
     *   by the module that did the original allocation because heaps
     *   are not necessarily shared. GetOrphanedScene() enforces you
     *   to delete the returned scene by yourself, but this will only
     *   be fine if and only if you're using the same heap as assimp.
     *   On Windows, it's typically fine provided everything is linked
     *   against the multithreaded-dll version of the runtime library.
     *   It will work as well for static linkage with Assimp.*/
    aiScene* GetOrphanedScene();

    // -------------------------------------------------------------------
    /** Returns whether a given file extension is supported by ASSIMP.
     *
     * @param szExtension Extension to be checked.
     *   Must include a trailing dot '.'. Example: ".3ds", ".md3".
     *   Cases-insensitive.
     * @return true if the extension is supported, false otherwise */
    bool IsExtensionSupported( const char* szExtension ) const;

    // -------------------------------------------------------------------
    /** @brief Returns whether a given file extension is supported by ASSIMP.
     *
     * This function is provided for backward compatibility.
     * See the const char* version for detailed and up-to-date docs.
     * @see IsExtensionSupported(const char*) */
    inline bool IsExtensionSupported( const std::string& szExtension ) const;

    // -------------------------------------------------------------------
    /** Get a full list of all file extensions supported by ASSIMP.
     *
     * If a file extension is contained in the list this does of course not
     * mean that ASSIMP is able to load all files with this extension ---
     * it simply means there is an importer loaded which claims to handle
     * files with this file extension.
     * @param szOut String to receive the extension list.
     *   Format of the list: "*.3ds;*.obj;*.dae". This is useful for
     *   use with the WinAPI call GetOpenFileName(Ex). */
    void GetExtensionList( aiString& szOut ) const;

    // -------------------------------------------------------------------
    /** @brief Get a full list of all file extensions supported by ASSIMP.
     *
     * This function is provided for backward compatibility.
     * See the aiString version for detailed and up-to-date docs.
     * @see GetExtensionList(aiString&)*/
    inline void GetExtensionList( std::string& szOut ) const;

    // -------------------------------------------------------------------
    /** Get the number of importers currently registered with Assimp. */
    size_t GetImporterCount() const;

    // -------------------------------------------------------------------
    /** Get meta data for the importer corresponding to a specific index..
     *
     *  For the declaration of #aiImporterDesc, include <assimp/importerdesc.h>.
     *  @param index Index to query, must be within [0,GetImporterCount())
     *  @return Importer meta data structure, nullptr if the index does not
     *     exist or if the importer doesn't offer meta information (
     *     importers may do this at the cost of being hated by their peers).*/
    const aiImporterDesc* GetImporterInfo( size_t index ) const;

    // -------------------------------------------------------------------
    /** Find the importer corresponding to a specific index.
     *
     *  @param index Index to query, must be within [0,GetImporterCount())
     *  @return Importer instance. nullptr if the index does not
     *     exist. */
    BaseImporter* GetImporter( size_t index ) const;

    // -------------------------------------------------------------------
    /** Find the importer corresponding to a specific file extension.
     *
     *  This is quite similar to #IsExtensionSupported except a
     *  BaseImporter instance is returned.
     *  @param szExtension Extension to check for. The following formats
     *    are recognized (BAH being the file extension): "BAH" (comparison
     *    is case-insensitive), ".bah", "*.bah" (wild card and dot
     *    characters at the beginning of the extension are skipped).
     *  @return nullptr if no importer is found*/
    BaseImporter* GetImporter( const char* szExtension ) const;

    // -------------------------------------------------------------------
    /** Find the importer index corresponding to a specific file extension.
     *
     *  @param szExtension Extension to check for. The following formats
     *    are recognized (BAH being the file extension): "BAH" (comparison
     *    is case-insensitive), ".bah", "*.bah" (wild card and dot
     *    characters at the beginning of the extension are skipped).
     *  @return (size_t)-1 if no importer is found */
    size_t GetImporterIndex( const char* szExtension ) const;

    // -------------------------------------------------------------------
    /** Returns the storage allocated by ASSIMP to hold the scene data
     * in memory.
     *
     * This refers to the currently loaded file, see #ReadFile().
     * @param in Data structure to be filled.
     * @note The returned memory statistics refer to the actual
     *   size of the use data of the aiScene. Heap-related overhead
     *   is (naturally) not included.*/
    void GetMemoryRequirements( aiMemoryInfo& in ) const;

    // -------------------------------------------------------------------
    /** Enables "extra verbose" mode.
     *
     * 'Extra verbose' means the data structure is validated after *every*
     * single post processing step to make sure everyone modifies the data
     * structure in a well-defined manner. This is a debug feature and not
     * intended for use in production environments. */
    void SetExtraVerbose( bool bDo );

    // -------------------------------------------------------------------
    /** Private, do not use. */
    ImporterPimpl* Pimpl() { return pimpl; }
    const ImporterPimpl* Pimpl() const { return pimpl; }

protected:
    // Just because we don't want you to know how we're hacking around.
    ImporterPimpl* pimpl;
};

} // namespace Assimp
